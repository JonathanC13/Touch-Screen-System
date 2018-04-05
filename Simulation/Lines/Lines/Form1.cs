using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Timers;
using System.Threading;
using System.Collections;
using System.Net;           // for 'IPAddress'
using System.Net.Sockets;   // for 'UdpClient'

using Microsoft.VisualBasic.PowerPacks;

namespace Lines
{

    public partial class Form1 : Form
    {
        private static int rows = 8;
        private static int cols = 8;
        //private List<Microsoft.VisualBasic.PowerPacks.LineShape> linesLst = new List<Microsoft.VisualBasic.PowerPacks.LineShape>();
        private LineShape[,] linesLst = new Microsoft.VisualBasic.PowerPacks.LineShape[rows, cols];
        private List<OvalShape> sensorLst = new List<Microsoft.VisualBasic.PowerPacks.OvalShape>();

        UdpClient client;

        // Creates and initializes a new Queue.
        Queue<byte[]> myQueue = new Queue<byte[]>();

        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            // Fill list with all lines
            //foreach (Microsoft.VisualBasic.PowerPacks.LineShape line in this.Controls.OfType<Microsoft.VisualBasic.PowerPacks.LineShape>())
            // {
            //   linesLst.Add(line);
            //}

            end_mutli_button.Enabled = false;

            // Lines for: -----
            // Emitter 1
            linesLst[0, 0] = E1_S1;
            linesLst[0, 1] = E1_S2;
            linesLst[0, 2] = E1_S3;
            linesLst[0, 3] = E1_S4;
            linesLst[0, 4] = E1_S5;
            linesLst[0, 5] = E1_S6;
            linesLst[0, 6] = E1_S7;
            linesLst[0, 7] = E1_S8;

            // Emitter 2
            linesLst[1, 0] = E2_S1;
            linesLst[1, 1] = E2_S2;
            linesLst[1, 2] = E2_S3;
            linesLst[1, 3] = E2_S4;
            linesLst[1, 4] = E2_S5;
            linesLst[1, 5] = E2_S6;
            linesLst[1, 6] = E2_S7;
            linesLst[1, 7] = E2_S8;

            // Emitter 3
            linesLst[2, 0] = E3_S1;
            linesLst[2, 1] = E3_S2;
            linesLst[2, 2] = E3_S3;
            linesLst[2, 3] = E3_S4;
            linesLst[2, 4] = E3_S5;
            linesLst[2, 5] = E3_S6;
            linesLst[2, 6] = E3_S7;
            linesLst[2, 7] = E3_S8;

            // Emitter 4
            linesLst[3, 0] = E4_S1;
            linesLst[3, 1] = E4_S2;
            linesLst[3, 2] = E4_S3;
            linesLst[3, 3] = E4_S4;
            linesLst[3, 4] = E4_S5;
            linesLst[3, 5] = E4_S6;
            linesLst[3, 6] = E4_S7;
            linesLst[3, 7] = E4_S8;

            // Emitter 5
            linesLst[4, 0] = E5_S1;
            linesLst[4, 1] = E5_S2;
            linesLst[4, 2] = E5_S3;
            linesLst[4, 3] = E5_S4;
            linesLst[4, 4] = E5_S5;
            linesLst[4, 5] = E5_S6;
            linesLst[4, 6] = E5_S7;
            linesLst[4, 7] = E5_S8;

            // Emitter 6
            linesLst[5, 0] = E6_S1;
            linesLst[5, 1] = E6_S2;
            linesLst[5, 2] = E6_S3;
            linesLst[5, 3] = E6_S4;
            linesLst[5, 4] = E6_S5;
            linesLst[5, 5] = E6_S6;
            linesLst[5, 6] = E6_S7;
            linesLst[5, 7] = E6_S8;

            // Emitter 7
            linesLst[6, 0] = E7_S1;
            linesLst[6, 1] = E7_S2;
            linesLst[6, 2] = E7_S3;
            linesLst[6, 3] = E7_S4;
            linesLst[6, 4] = E7_S5;
            linesLst[6, 5] = E7_S6;
            linesLst[6, 6] = E7_S7;
            linesLst[6, 7] = E7_S8;

            // Emitter 8
            linesLst[7, 0] = E8_S1;
            linesLst[7, 1] = E8_S2;
            linesLst[7, 2] = E8_S3;
            linesLst[7, 3] = E8_S4;
            linesLst[7, 4] = E8_S5;
            linesLst[7, 5] = E8_S6;
            linesLst[7, 6] = E8_S7;
            linesLst[7, 7] = E8_S8;
            // lines end -----

            // Sensors start -----
            sensorLst.Add(Sensor1);
            sensorLst.Add(Sensor2);
            sensorLst.Add(Sensor3);
            sensorLst.Add(Sensor4);
            sensorLst.Add(Sensor5);
            sensorLst.Add(Sensor6);
            sensorLst.Add(Sensor7);
            sensorLst.Add(Sensor8);
            // Sensors end -----
            
        }

        private void Form1_Shown(object sender, EventArgs e)
        {
           
        }

        // Turn OFF all emitters
        private void button_all_off_Click(object sender, EventArgs e)
        {
            // Change "visible" of all emitters to false;
            for (int r = 0; r < rows; r++)
            {
                for (int c = 0; c < cols; c++)
                {
                    Microsoft.VisualBasic.PowerPacks.LineShape currLine = linesLst[r, c];
                    currLine.Visible = false;                   
                }
            }
            /*
                for (int x = 0; x < linesLst.Count; x++)
                {
                    Microsoft.VisualBasic.PowerPacks.LineShape currLine = linesLst[x];
                    currLine.Visible = false;
                }
            */
        }

        // Turn ON all emitters
        private void button_all_ON_Click(object sender, EventArgs e)
        {
            // Change "visible" of all emitters to true;
            for (int r = 0; r < rows; r++)
            {
                for (int c = 0; c < cols; c++)
                {
                    Microsoft.VisualBasic.PowerPacks.LineShape currLine = linesLst[r, c];
                    currLine.Visible = true;
                }
            }
            /*
            // Change "visible" of all emitters to true;
            for (int x = 0; x < linesLst.Count; x++) {
               
                Microsoft.VisualBasic.PowerPacks.LineShape currLine = linesLst[x];
                currLine.Visible = true;
            }
            */
        }
        static System.Windows.Forms.Timer myTimer = new System.Windows.Forms.Timer();
        static bool exitFlag = false;

        private async void start_op_Click(object sender, EventArgs e)
        {
            // clear window
            button_all_off_Click(sender, e);

            // Turn on one emitter in order while only one can be on at a time
            int currEmitter = 0;

            while (true)
            {                
                // Turn off lines that don't belong to the current emitter
                for (int i = 0; i < rows; i++)
                {
                    for (int j = 0; j < cols; j++)
                    {
                        Microsoft.VisualBasic.PowerPacks.LineShape currLine = linesLst[i, j];
                        currLine.Visible = false;
                    }               
                }

                // Turn on the current emitter lines
                for (int p = 0; p < cols; p++)
                {
                    Microsoft.VisualBasic.PowerPacks.LineShape currLine = linesLst[currEmitter, p];
                    currLine.Visible = true;
                }

                // exit condition check
                if (exitFlag == true)
                {
                    break;
                }

                await Task.Delay(1000);
                // next emitter
                if (currEmitter + 1 <= 7)
                {
                    currEmitter = currEmitter + 1;           
                }
                else
                {
                    currEmitter = 0;                    
                }

                if (exitFlag == true)
                {                    
                    break;
                }

            }
            // reset the exitFlag
            exitFlag = false;

            // --
            button_all_ON_Click(sender, e);

        }                

        private void stop_seq_Click(object sender, EventArgs e)
        {
            exitFlag = true;
        }

        private void default_button_Click(object sender, EventArgs e)
        {
            // Change location of all sensors to 1130, max fit
            for (int x = 0; x < sensorLst.Count; x++)
            {

                Microsoft.VisualBasic.PowerPacks.OvalShape currLine = sensorLst[x];
                currLine.Left = 1130;
            }
            
            // Change x2 of all lines to 1139 to match the sensors
            for (int x = 0; x < rows; x++)
            {
                for (int y = 0; y < cols; y++)
                {
                    Microsoft.VisualBasic.PowerPacks.LineShape currLine = linesLst[x, y];
                    currLine.X2 = 1139;
                }                
            }
        }

        private void decrease_button_Click(object sender, EventArgs e)
        {
            // Decrement x co-ordinate of the sensors to the left
            for (int x = 0; x < sensorLst.Count; x++)
            {                            
                Microsoft.VisualBasic.PowerPacks.OvalShape currSensor = sensorLst[x];
                // Check if passes Lower limit of x = 18, if true break.
                if (currSensor.Left - 20 < 18)
                {
                    break;
                }
                else
                {
                    currSensor.Left -= 20;
                }
            }

            // Decrement x co-ordinate of the lines to match the sensors
            for (int x = 0; x < rows; x++)
            {
                for (int y = 0; y < cols; y++)
                {
                    Microsoft.VisualBasic.PowerPacks.LineShape currLine = linesLst[x, y];

                    // Check if passes lower limit of x = 18, if true break.
                    if (currLine.X2 - 20 < 27)
                    {
                        break;
                    }
                    else
                    {
                        currLine.X2 -= 20;
                    }
                }                                
            }
        }

        private void increase_button_Click(object sender, EventArgs e)
        {
            // Increment x co-ordinate of the sensors to the right
            for (int x = 0; x < sensorLst.Count; x++)
            {
                Microsoft.VisualBasic.PowerPacks.OvalShape currSensor = sensorLst[x];
                // Check if passes upper limit of x = 1130, if true break.
                if (currSensor.Left + 20 > 1130)
                {
                    break;
                }
                else
                {
                    currSensor.Left += 20;
                }
            }

            // Increment x co-ordinate of the lines to match the sensors
            for (int x = 0; x < rows; x++)
            {
                for (int y = 0; y < cols; y++)
                {
                    Microsoft.VisualBasic.PowerPacks.LineShape currLine = linesLst[x, y];
                    // Check if at upper limit of x = 1139, if true break.
                    if (currLine.X2 + 20 > 1139)
                    {
                        break;
                    }
                    else
                    {
                        currLine.X2 += 20;
                    }
                }                
            }
        }

        // End thread flag
        private bool endThread = false;

        // When clicked, it started the thread to receive multicast messages from the rpi
        private void start_multi_button_Click(object sender, EventArgs e)
        {
            button_all_off_Click(sender, e);
            start_multi_button.Enabled = false;

            ThreadStart multi = new ThreadStart(multicastReceiver_Thread);
            Thread piThread = new Thread(multi);
            piThread.Start();
            // Click handler returns so the form can continue
        }

        // Receives the multicast message and fills a queue.
        private void multicastReceiver_Thread()
        {
            client = new UdpClient();

            client.ExclusiveAddressUse = false;
            IPEndPoint localEp = new IPEndPoint(IPAddress.Any, 16789);

            // Socket methods Socket sock = client.Client
            client.Client.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReuseAddress, true);
            client.Client.Bind(localEp);

            IPAddress multicastaddress = IPAddress.Parse("224.0.0.1");
            client.JoinMulticastGroup(multicastaddress);

            Console.WriteLine("Listening on port, click 'End multicast receiver' button to quit.");

            while (endThread == false)
            {
                try
                {
                    Byte[] data = client.Receive(ref localEp);
                    myQueue.Enqueue(data);

                    Console.Write("Got data, bytecount = " + data[0]);
                } catch(Exception e){

                }
            }

            // close client
            client.Close();

            // reset flag
            endThread = false;
            MessageBox.Show("Clicked");
        } // End of multicast thread

        private void end_mutli_button_Click(object sender, EventArgs e)
        {
            end_mutli_button.Enabled = false;
            endThread = true;
        }

        const int MCIDX_HEADER = 0;
        const int MCIDX_TIME_T = 4;

        const int MCIDX_X_INTERCEPT = 8;
        const int MCIDX_Y_INTERCEPT = 12;

        const int MCIDX_IR0_SENSOR_ST = 16;
        const int MCIDX_IR1_SENSOR_ST = 20;
        const int MCIDX_IR2_SENSOR_ST = 24;
        const int MCIDX_IR3_SENSOR_ST = 28;
        const int MCIDX_IR4_SENSOR_ST = 32;
        const int MCIDX_IR5_SENSOR_ST = 36;
        const int MCIDX_IR6_SENSOR_ST = 40;
        const int MCIDX_IR7_SENSOR_ST = 44;

        const int MCIDX_GEN_TEXT = 48;

        private int cnt = 0;

        private void checkQ_Tick(object sender, EventArgs e)
        {
            if (myQueue.Count != 0)
            {
                byte[] bytes = myQueue.Dequeue();

                Console.WriteLine("Received Queue Data from Multicast.\n");

                uint header = BitConverter.ToUInt32(bytes, MCIDX_HEADER + 4);   //add 4 because there is a 4-byte byte count before the body

                Console.WriteLine("Header = " + header);

                uint time = BitConverter.ToUInt32(bytes, MCIDX_TIME_T + 4);

                Console.WriteLine("Time = " + header);

                float xvalue = BitConverter.ToSingle(bytes, MCIDX_X_INTERCEPT + 4);

                Console.WriteLine("x intercept = " + xvalue);

                float yvalue = BitConverter.ToSingle(bytes, MCIDX_Y_INTERCEPT + 4);

                Console.WriteLine("x intercept = " + yvalue);

                uint[] sensor_state = new uint[8];

                for (int i = 0; i < 8; i++)
                {
                    sensor_state[i] = BitConverter.ToUInt32(bytes, MCIDX_IR0_SENSOR_ST + (i * 4) + 4);

                    Console.WriteLine("ir #" + i + " sensor = " + sensor_state[i]);
                }

                for (int led = 0; led < 8; led++)
                {
                    for (int sens = 0; sens < 8; sens++) // 8 sensors
                    {
                        int mask = (1 << sens);
                        int bit = (int) (sensor_state[led] & mask) & 0xff;
                        if (bit == 0)
                        {                           
                            Microsoft.VisualBasic.PowerPacks.LineShape currLine = linesLst[led, sens];
                            currLine.Visible = true;          
                        }
                    }
                    
                }

                /*
                byte[] all_pairs = new byte[32];
                
                // Only pair sensor information for each emitter
                Buffer.BlockCopy(bytes, 20, all_pairs, 0, 32);
                int emitter_byte_offset = 4;
                int sensor_offset;

                // Toggle correct lines, "i" is the emitter

                Console.WriteLine(" pairs on form ------");
                // Loop for number of emitters
                for (int i = 0; i < rows; i++)
                {
                    sensor_offset = 0;
                    // Loop for number of bytes for one emitter, which is also the number of possible sensors.
                    for (int j = 0; j < 4; j++)
                    {
                        byte pairs = all_pairs[j + (i * emitter_byte_offset)];
                        // Loop for checking bit by bit in a byte
                        for( int k = 0; k < 8; k ++){
                            var bit = pairs & (1 << j);
                            if (bit == 0)
                            {                                                               
                                Console.WriteLine("Emitter: " + i + " with off sensor: " + j + sensor_offset);
                                Microsoft.VisualBasic.PowerPacks.LineShape currLine = linesLst[i, j + sensor_offset];
                                currLine.Visible = true;                                
                            }
                        }
                        sensor_offset = sensor_offset + 8;
                    }

                    sensor_state[i] = BitConverter.ToUInt32(bytes, MCIDX_IR0_SENSOR_ST + (i * 4) + 4);
                    Console.WriteLine("\n");
                }
                Console.WriteLine(" pairs end -----");
                 * */

              
            }
            else
            {
                // nothing
            }
        }

        private void test_button_Click(object sender, EventArgs e)
        {
            byte[] testByte = new byte[80];
            button_all_off_Click(sender, e);
            //button_all_ON_Click(sender, e);

            testByte[MCIDX_IR0_SENSOR_ST + 4] = 0xfb;
            testByte[MCIDX_IR0_SENSOR_ST + 4 + 1] = 0xff;
            testByte[MCIDX_IR0_SENSOR_ST + 4 + 2] = 0xff;
            testByte[MCIDX_IR0_SENSOR_ST + 4 + 3] = 0xff;

            testByte[MCIDX_IR1_SENSOR_ST + 4] = 0xfd;
            testByte[MCIDX_IR1_SENSOR_ST + 4 + 1] = 0xff;
            testByte[MCIDX_IR1_SENSOR_ST + 4 + 2] = 0xff;
            testByte[MCIDX_IR1_SENSOR_ST + 4 + 3] = 0xff;

            testByte[MCIDX_IR2_SENSOR_ST + 4] = 0xfe;
            testByte[MCIDX_IR2_SENSOR_ST + 4 + 1] = 0xff;
            testByte[MCIDX_IR2_SENSOR_ST + 4 + 2] = 0xff;
            testByte[MCIDX_IR2_SENSOR_ST + 4 + 3] = 0xff;

            testByte[MCIDX_IR3_SENSOR_ST + 4] = 0xff;
            testByte[MCIDX_IR3_SENSOR_ST + 4 + 1] = 0xff;
            testByte[MCIDX_IR3_SENSOR_ST + 4 + 2] = 0xff;
            testByte[MCIDX_IR3_SENSOR_ST + 4 + 3] = 0xff;

            testByte[MCIDX_IR4_SENSOR_ST + 4] = 0xff;
            testByte[MCIDX_IR4_SENSOR_ST + 4 + 1] = 0xff;
            testByte[MCIDX_IR4_SENSOR_ST + 4 + 2] = 0xff;
            testByte[MCIDX_IR4_SENSOR_ST + 4 + 3] = 0xff;

            testByte[MCIDX_IR5_SENSOR_ST + 4] = 0xff;
            testByte[MCIDX_IR5_SENSOR_ST + 4 + 1] = 0xff;
            testByte[MCIDX_IR5_SENSOR_ST + 4 + 2] = 0xff;
            testByte[MCIDX_IR5_SENSOR_ST + 4 + 3] = 0xff;

            testByte[MCIDX_IR6_SENSOR_ST + 4] = 0xff;
            testByte[MCIDX_IR6_SENSOR_ST + 4 + 1] = 0xff;
            testByte[MCIDX_IR6_SENSOR_ST + 4 + 2] = 0xff;
            testByte[MCIDX_IR6_SENSOR_ST + 4 + 3] = 0xff;

            testByte[MCIDX_IR7_SENSOR_ST + 4] = 0xff;
            testByte[MCIDX_IR7_SENSOR_ST + 4 + 1] = 0xff;
            testByte[MCIDX_IR7_SENSOR_ST + 4 + 2] = 0xff;
            testByte[MCIDX_IR7_SENSOR_ST + 4 + 3] = 0xff;


            myQueue.Enqueue(testByte);
        }

        private void label3_Click(object sender, EventArgs e)
        {

        }


    }
}
