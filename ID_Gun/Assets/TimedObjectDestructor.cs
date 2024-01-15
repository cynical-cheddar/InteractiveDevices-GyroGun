using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class TimedObjectDestructor : MonoBehaviour
{
    public float t = 1;
    // Start is called before the first frame update
    void Start()
    {
        Invoke(nameof(Die), t);
    }

    void Die()
    {
        Destroy(gameObject);
    }


}
