using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Health : MonoBehaviour
{
    // Start is called before the first frame update

    public float hitPoints = 20f;

    public GameObject debris;


    void Start()
    {
        
    }

    public void ApplyDamage(float dmg)
    {
        hitPoints -= dmg;
        if(hitPoints <= 0)
        {
            Instantiate(debris, transform.position, transform.rotation);
            Destroy(gameObject);
        }
    }
}
