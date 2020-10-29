#include "../headers/cluster_lloyd.h"

void Lloyd_Cluster::Lloyd_Clustering()
{
    //Important variables..
    float ratio;
    double clustering_time=0.0;

    //Original array of kmeans centroids...
    int* indexes = kmeansptr->get_centroids(),iters=0;

    //Store previous and current objective value in this array..
    float objectives_values[2];
    objectives_values[0]=100;
    objectives_values[1]=100;

    //Allocate memory for array with silhouette values...
    float* silhouette_array = new float[kmeansptr->get_K()+1];    
    for(int k=0;k<=kmeansptr->get_K();k++)  silhouette_array[k]=0.0;

    //Store centroids with results of kmeans++ initialization...
    for(int i=0;i<kmeansptr->get_K();i++)
    {
        for(int j=0;j<kmeansptr->get_dimensions();j++)
            centroids[i][j] = kmeansptr->get_Images_Array()[indexes[i]][j];
    }
    file << endl;
    
    auto start = chrono::high_resolution_clock::now();
    while(true)
    {
        Lloyd_Assign();

        objectives_values[1] = Lloyd_Objective();
        ratio = abs(objectives_values[1]-objectives_values[0])/objectives_values[0];
        objectives_values[0] = objectives_values[1];
        file << "Reduction's rate change of objective function's value: " << ratio << endl;
        if(ratio<epsilon)   break;
        
        Lloyd_Update();
        iters++;
    }
    auto end = chrono::high_resolution_clock::now(); 
    clustering_time = chrono::duration_cast<chrono::seconds>(end - start).count();
    file << endl << "Converged with " << iters << " updates!" << endl;

    Silhouette(&points,kmeansptr->get_K(),&silhouette_array,kmeansptr);
    
    Lloyd_Print(silhouette_array,(clustering_time+kmeansptr->get_kmeans_time()));

    delete [] silhouette_array;
}

void Lloyd_Cluster::Lloyd_Assign()
{
    int dist1,n1,dist2,n2;

    //For each point of dataset keep 2 nearest centroids and the appropriate distances in our map...
    for(int i=0;i<kmeansptr->get_number_of_images();i++)
    {
        //Store pairs of distance to each centroid from a point of dataset and index of this centroid...
        priority_queue<pair<int,int>, vector<pair<int,int>>, greater<pair<int,int>> > distances; 
        
        for(int j=0;j<kmeansptr->get_K();j++)
            distances.push(make_pair(ManhattanDistance(kmeansptr->get_Images_Array()[i],centroids[j],kmeansptr->get_dimensions()),j));
        
        //Store distance and index of first nearest centroid...
        dist1 = distances.top().first;
        n1 = distances.top().second;
        
        //Pop up first pair...
        distances.pop();
        
        //Store distance and index of second nearest centroid...
        dist2 = distances.top().first;
        n2 = distances.top().second;

        //Pass these values to class Nearest_Centroids of each point of dataset...
        points[i]->set_dist1(dist1);
        points[i]->set_nearest_centroid1(n1);
        points[i]->set_dist2(dist2);
        points[i]->set_nearest_centroid2(n2);
    }
}

void Lloyd_Cluster::Lloyd_Update()
{    
    map <int,Nearest_Centroids*>::iterator it;
    int cluster=0,median_index=0;

    // //Initialize K*dimensions vectors...
    // vector<item>** vectors = new vector<item>*[kmeansptr->get_K()];
    // for(int i=0;i<kmeansptr->get_K();i++)   vectors[i] = new vector<item>[kmeansptr->get_dimensions()]; 

    // //Fill vectors with features of each image of dataset...
    // for(it=points.begin();it!=points.end();it++)    
    // {
    //     cluster = it->second->get_nearest_centroid1();
    //     for(int z=0;z<kmeansptr->get_dimensions();z++)
    //         vectors[cluster][z].push_back(kmeansptr->get_Images_Array()[it->first][z]);
    // }

    // //Sort each vector and choose the appropriate feature (with median index) 
    // //in order to have a new one centroid.
    // for(int i=0;i<kmeansptr->get_K();i++)   
    // {
    //     for(int z=0;z<kmeansptr->get_dimensions();z++)
    //     {
    //         if(vectors[i][z].size()!=0)
    //         {
    //             sort(vectors[i][z].begin(),vectors[i][z].end());
    //             median_index = vectors[i][z].size()/2; 
    //             centroids[i][z] = vectors[i][z][median_index];
    //         }
    //     }
    // }
    // 
    // //Deallocate memory for vectors...
    // for(int i=0;i<kmeansptr->get_K();i++)   delete [] vectors[i];
    // delete [] vectors;   

    vector <item> vec;

    for(int i=0;i<kmeansptr->get_K();i++)   
    {
        for(int z=0;z<kmeansptr->get_dimensions();z++)
        {
            vec.clear();

            for(it=points.begin();it!=points.end();it++)    
            {
                cluster = it->second->get_nearest_centroid1();
                if(cluster==i)
                    vec.push_back(kmeansptr->get_Images_Array()[it->first][z]);
            }

            if(vec.size()!=0)
            {
                sort(vec.begin(),vec.end());
                median_index = vec.size()/2; 
                centroids[i][z] = vec[median_index];
            }
        }
    } 
}

float Lloyd_Cluster::Lloyd_Objective()
{
    float avg_sum=0.0;
    int K=kmeansptr->get_K(),cluster=0,sums[K];
    for(int i=0;i<kmeansptr->get_K();i++)   sums[i]=0;

    map <int,Nearest_Centroids*>::iterator it;

    for(it=points.begin();it!=points.end();it++)    
    {
        cluster = it->second->get_nearest_centroid1();
        sums[cluster] += ManhattanDistance(kmeansptr->get_Images_Array()[it->first],centroids[cluster],kmeansptr->get_dimensions());
    }
    
    for(int i=0;i<K;i++)    avg_sum+=sums[i];

    return avg_sum;
}

void Lloyd_Cluster::Lloyd_Print(float* silhouette_array,int time)
{
    //Declaration of important structures,variables...
    int cluster=0,K=kmeansptr->get_K();
    map <int,Nearest_Centroids*>::iterator it;
    int images_in_cluster[K];
    for(int i=0;i<kmeansptr->get_K();i++)   images_in_cluster[i]=0;

    //Iterate whole map so as to calculate silhouette for points of each cluster...
    for(it=points.begin();it!=points.end();it++)    
    {
        cluster = it->second->get_nearest_centroid1();
        images_in_cluster[cluster]++;
    }

    file << endl << "Algorithm: Lloyds" << endl;
    
    for(int i=0;i<K;i++)   
    {
        file << "CLUSTER-" << i << "  {" << images_in_cluster[i] << ", [ ";
        for(int z=0;z<kmeansptr->get_dimensions();z++)  file << centroids[i][z] << " "; 
        file << "]}" << endl << endl;
    }
    
    file << "clustering_time: " << time << "s" << endl << endl << "Silhouette: [ ";

    for(int i=0;i<=K;i++)   
    {
        file << fixed << setprecision(3) << silhouette_array[i];
        if(i<K) file << ", ";
        else    file << " ]";
    }
    file << endl << endl;

    if(complete=="yes")
    {
        for(int i=0;i<K;i++)   
        {
            file << "CLUSTER-" << i << "{ [ ";
            for(int z=0;z<kmeansptr->get_dimensions();z++)  file << centroids[i][z] << " "; 
            file << "], Images: ";

            for(it=points.begin();it!=points.end();it++)    
            {
                cluster = it->second->get_nearest_centroid1();
                if(cluster==i)  file << it->first << ", "; 
            }
            file << "}" << endl << endl;
        }
    }
}