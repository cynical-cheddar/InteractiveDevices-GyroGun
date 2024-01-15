using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Audio;

public class VolumeControl : MonoBehaviour
{
    [Range(0.05f, 1.0f)]
    [SerializeField] public float weaponsAndExplosionsVolume = 0.4f;

    public AudioMixer weaponsAndExplosions;

    // Start is called before the first frame update
    void Start()
    {
        weaponsAndExplosions.SetFloat("sfxVolume", Mathf.Log10(weaponsAndExplosionsVolume) * 20);
    }

    // Update is called once per frame
    void Update()
    {
        
    }
}
