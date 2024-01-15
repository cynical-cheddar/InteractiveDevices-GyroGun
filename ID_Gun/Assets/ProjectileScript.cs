using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class ProjectileScript : MonoBehaviour
{

    public float damage = 10f;
    public GameObject impactFx;

    Vector3 startPos = Vector3.zero;

    private void Start()
    {
        startPos = transform.position;
    }
    // Start is called before the first frame update
    private void OnCollisionEnter(Collision collision)
    {
        if(collision.gameObject.GetComponent<Health>() != null)
        {
            collision.gameObject.GetComponent<Health>().ApplyDamage(damage);
        }

        GameObject impact = Instantiate(impactFx, transform.position, Quaternion.identity);
        impact.transform.LookAt(startPos);
        Destroy(gameObject);

    }
}
