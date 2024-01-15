using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class GunController : MonoBehaviour
{

    public float damage = 10f;

    public GameObject projectilePrefab;

    public Transform barrel;

    public float projSpeed = 10f;

    public GameObject muzzleFlashPrefab;


    Quaternion oldRotation = Quaternion.identity;
    Quaternion newRotation = Quaternion.identity;

    public float cooldown = 0.5f;
    float currentCooldown = 0f;

    public Animation animation;


    public int ammoMax = 5;
    public int ammo = 5;

    public AudioClip reloadSound;

    public float interpPeriod = 0.1f;
    float interpStart = 0;
    // Start is called before the first frame update
    void Start()
    {
        interpStart = Time.time;
    }

    public void Reload()
    {
        ammo = ammoMax;
        GetComponent<AudioSource>().PlayOneShot(reloadSound);
    }

    public void SetOrientations(Quaternion oldRot, Quaternion newRot)
    {
        oldRotation = oldRot;
        newRotation = newRot;
        interpStart = Time.time;
    }

    private void Update()
    {
        float t = (Time.time - interpStart) / interpPeriod;
        if (t > 1) t = 1;
        transform.rotation = Quaternion.Lerp(oldRotation, newRotation, t);

        currentCooldown -= Time.deltaTime;
    }

    public bool CanFire()
    {
        if (currentCooldown <= 0 && ammo > 0)
        {
            return true;
        }
        else return false;
        
    }

    public void FireDelay(float t)
    {
        //ammo--;
        currentCooldown = cooldown;
        Invoke(nameof(Fire), t);
    }
    public void Fire()
    {
        
        animation.Play();
        currentCooldown = cooldown;
        GameObject proj = Instantiate(projectilePrefab, barrel.transform.position, barrel.transform.rotation);
        ProjectileScript ps = proj.GetComponent<ProjectileScript>();
        proj.GetComponent<Rigidbody>().AddForce(proj.transform.forward * projSpeed, ForceMode.VelocityChange);

        GameObject mf = Instantiate(muzzleFlashPrefab, barrel.transform.position, barrel.transform.rotation);
        
    }

}
