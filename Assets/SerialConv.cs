using UnityEngine;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.Linq;
using System.Text;

public class SerialConv : MonoBehaviour
{

    [DllImport("serialconv", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
    static extern int scvGetSerialCOM(string vId, string pId, [MarshalAs(UnmanagedType.LPStr)]System.Text.StringBuilder deviceInfo, int buffer_size);

    public string sampleVID;
    public string samplePID;

    // Start is called before the first frame update
    void Start()
    {
        const int buffersize = 1024;
        System.Text.StringBuilder deviceString = new System.Text.StringBuilder(buffersize);
        int num = scvGetSerialCOM(sampleVID,samplePID,deviceString,buffersize);
        string comList = deviceString.ToString().TrimEnd(';');

        if (comList != "")
        {
            string[] portsName = comList.Split(';');

            //GET
            foreach (string port in portsName)
                Debug.Log(port);
        }
    }

}
