using UnityEngine;
using System.Collections;
using System.IO.Ports;
using System;
using System.Text;
using UnityEngine.UI;

public class SerialDuplexManager : MonoBehaviour
{
    SerialController serialController;

    public Queue incomingMessageBuffer = new Queue();

    // Initialization
    void Start()
    {
        serialController = FindObjectOfType<SerialController>();
    }

    // send message to dongle with this method
    public void SendMessageViaSerial(string message)
    {
        serialController.SendSerialMessage(message);
    }

    // Executed each frame
    void Update()
    {


        // read incoming messages

        string message = serialController.ReadSerialMessage();

        if (message == null)
            return;

        // Check if the message is plain data or a connect/disconnect event.

        // if the incoming message is a device connected notification
        if (ReferenceEquals(message, SerialController.SERIAL_DEVICE_CONNECTED))
            Debug.Log("Connection established");
        // if the incoming message is a device disconnected notification
        else if (ReferenceEquals(message, SerialController.SERIAL_DEVICE_DISCONNECTED))
            Debug.Log("Connection attempt failed or disconnection detected");
        // otherwise, we have a genuine message
        else
            //Debug.Log("Message arrived: " + message + "   -  adding to buffer");
            incomingMessageBuffer.Enqueue(message);

    }
}