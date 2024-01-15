/**
 * Ardity (Serial Communication for Arduino + Unity)
 * Author: Daniel Wilches <dwilches@gmail.com>
 *
 * This work is released under the Creative Commons Attributions license.
 * https://creativecommons.org/licenses/by/2.0/
 */

using UnityEngine;
using System.Collections;

/**
 * Sample for reading using polling by yourself, and writing too.
 */
public class SampleUserPolling_ReadWrite : MonoBehaviour
{
    public SerialController serialController;

    public Transform gun;

    public GunController gunController;

    // Initialization
    void Start()
    {
        serialController = GameObject.Find("SerialController").GetComponent<SerialController>();

        Debug.Log("Press A or Z to execute some actions");

        StartCoroutine(nameof(ReadCoroutine));
    }

    public IEnumerator ReadCoroutine()
    {
        while (true)
        {
            ReadData();
            yield return new WaitForSeconds(0.001f);
        }
    }

    // Executed each frame
    void Update()
    {
        //---------------------------------------------------------------------
        // Send data
        //---------------------------------------------------------------------

        // If you press one of these keys send it to the serial device. A
        // sample serial device that accepts this input is given in the README.
        if (Input.GetKeyDown(KeyCode.A))
        {
            Debug.Log("Sending @F");
            serialController.SendSerialMessage("@F");
        }

        if (Input.GetKeyDown(KeyCode.Z))
        {
            Debug.Log("Sending Z");
            serialController.SendSerialMessage("Z");
        }


        //---------------------------------------------------------------------
        // Receive data
        //---------------------------------------------------------------------


    }

    void ReadData()
    {
        string message = serialController.ReadSerialMessage();

        if (message == null)
            return;

        if (message == "")
        {
            Debug.Log("empty message");
            return;
        }

        // Check if the message is plain data or a connect/disconnect event.
        if (ReferenceEquals(message, SerialController.SERIAL_DEVICE_CONNECTED))
            Debug.Log("Connection established");
        else if (ReferenceEquals(message, SerialController.SERIAL_DEVICE_DISCONNECTED))
            Debug.Log("Connection attempt failed or disconnection detected");
        else
            Debug.Log("Message arrived: " + message);

        if (message == "@F")
        {
            if (gunController.CanFire())
            {
                serialController.SendSerialMessage("@F");
                gunController.FireDelay(0.1f);
            }
        }
        if (message == "@L")
        {
            gunController.Reload();
            serialController.SendSerialMessage("@L");
        }

        if (message[0] == '#')
        {
            //Debug.Log(message);
            string telemetry = message.Remove(0, 1);
            string[] orientations = telemetry.Split(',');
            //Debug.Log(telemetry);
            int x = int.Parse(orientations[0]);
            int y = int.Parse(orientations[1]);
            int z = int.Parse(orientations[2]);
            // if 2d
            Quaternion newOrientation = Quaternion.Euler(x / 2, -z, 0);
            Quaternion oldOrientation = gun.rotation;

            gun.GetComponent<GunController>().SetOrientations(oldOrientation, newOrientation);

           // gun.rotation = Quaternion.Euler(x / 2, -z, 0);
        }
    }
}