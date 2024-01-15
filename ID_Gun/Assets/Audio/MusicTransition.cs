using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.SceneManagement;



 public class MusicTransition : MonoBehaviour
 {
     
     public List<string> sceneNames; 
    
     public string instanceName;
 

     private void Start()
     {
         DontDestroyOnLoad(this.gameObject);

         SceneManager.sceneLoaded += OnSceneLoaded;
     }
 
     void OnSceneLoaded(Scene scene, LoadSceneMode mode)
     {
         CheckForDuplicateInstances();

         CheckIfSceneInList();
     }
 
     void CheckForDuplicateInstances()
     {
         // cache all objects containing this component
         MusicTransition[] collection = FindObjectsOfType<MusicTransition>();
 
         // iterate through the objects with this component, deleting those with matching identifiers
         foreach (MusicTransition obj in collection)
         {
             if(obj != this) // avoid deleting the object running this check
             {
                 if (obj.instanceName == instanceName)
                 {
                     Debug.Log("Duplicate object in loaded scene, deleting now...");
                     DestroyImmediate(obj.gameObject);
                 }
             }
         }
     }
 
     void CheckIfSceneInList()
     {
         // check what scene we are in and compare it to the list of strings 
         string currentScene = SceneManager.GetActiveScene().name;
 
         if (!sceneNames.Contains(currentScene))
         {
              // unsubscribe to the scene load callback
             SceneManager.sceneLoaded -= OnSceneLoaded;
             DestroyImmediate(this.gameObject);
         }
     }
 }