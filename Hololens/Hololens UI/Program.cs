using System;
using Windows.ApplicationModel.Core;
using Windows.Media.Capture;
using Windows.ApplicationModel;
using System.Threading.Tasks;
using Windows.System.Display;
using Windows.Graphics.Display;

using Urho;
using Urho.SharpReality;
using Urho.Gui;

using RabbitMQ.Client;
using RabbitMQ.Client.Events;
using System.Text;
using System.Diagnostics;

using System.Collections.Generic;
using Newtonsoft.Json;

namespace Hololens_UI
{
    internal class Program
    {
        [MTAThread]
        static void Main() => CoreApplication.Run(
            new UrhoAppViewSource<HelloWorldApplication>(
                new ApplicationOptions("Data")));
    }

    public class HelloWorldApplication : StereoApplication
    {
        //Deklarasi variabel global
        Text yawText;
        Text pitchText;
        Text readyText;
        Text positionText;
        Text headingText;

        Sprite crosshair; //format crosshair

        int tembak; //keterangan turret sedang menembak

        //int tlx, tly, brx, bry, sum_box;
        //Titik tlx, tly, brx, bry, sum_box;

        int sum_box_lama = 0;
        int sum_box = 0;
        float timesec = 0;

        bool ada_isi_awal = false;


        //bool TrackingMode = false;
        //string TrackingModeString = "0";
        int indBoxTrackedPlusOne = 0;


        //deklarasi penggunaan RabbitMQ
        RMQ rmq = new RMQ();

        //buat struct koordinat
        public struct koor
        {
            public int x, y;

            public koor(int p1, int p2)
            {
                x = p1;
                y = p2;
            }
        }

        //alamat tujuan
        string ex_holotoraspi = "ex_holotoraspi";
        string rk_holotoraspi = "rk_holotoraspi";
        string holotoraspi = "holotoraspi";
        string ex_TrackingMode = "ex_TrackingMode";
        string queue_TrackingMode = "queue_TrackingMode";
        string rk_TrackingMode = "rk_TrackingMode";




        //alamat asal
        string raspitoholo = "raspitoholo";
        string servertoholo = "servertoholo";

        //alamat nodemcu
        string rk_mcu_intopic = "inTopic";
        string queue_mcu_intopic = "inTopic";
        string queue_mcu_outtopic = "outTopic";
        string ex_mcu = "amq.topic";

        List<KotakTarget> bounding_box = new List<KotakTarget>();
        List<int> tlx = new List<int>();
        List<int> tly = new List<int>();
        List<int> brx = new List<int>();
        List<int> bry = new List<int>();

        //List<KotakTarget> listofBoundingBox;

        public HelloWorldApplication(ApplicationOptions opts) : base(opts) { }

        protected override void Start()
        //program Start
        {
            // base.Start() creates a basic scene
            base.Start();



            EnableGestureTapped = true;
            ada_isi_awal = false;

            //Tampilkan crosshair
            ShowCrosshair();

            //Tampilkan sudut Yaw & Pitch
            ShowYaw();
            ShowPitch();

            //Tampilkan informasi turret dari sensor
            ShowReady();
            ShowHeading();
            ShowPosition();

            bounding_box.Add(new KotakTarget(56, -91, 274, 25, UI.Root, Context, ResourceCache.GetTexture2D(@"2D/redbox.png")));

            Debug.WriteLine(bounding_box[0] is KotakTarget);

            //bounding_box[0] =  new KotakTarget(  56, -91, 274, 25, UI.Root, Context, ResourceCache.GetTexture2D(@"2D/redbox.png"));
            //Tampilkan bounding box
            //bounding_box[0].tlx = 696; //= 696;
            //bounding_box[0].tly = 269; //= 269;
            //bounding_box[0].brx = 914; //= 914;
            //bounding_box[0].bry = 385; //= 385;

            //CreateBoundingBox(tlx, tly, brx, bry);



            //Debug.WriteLine("Create boduning box");
            //Debug.WriteLine("hasil : " + tlx, " : ", tly, " : ", brx, " : ", bry);

            bounding_box[0].Delete();
            bounding_box.Remove(bounding_box[0]);


            Debug.WriteLine("Jumlah bounding box = {0}", bounding_box.Count);
            //bounding_box.RemoveAt(0);
            //Debug.WriteLine(bounding_box[0] is KotakTarget);

            //setting rabbitmq
            rmq.InitRMQConnection();
            rmq.CreateRMQConnection();

            rmq.CreateRMQChannel(raspitoholo,
                                    servertoholo,
                                    holotoraspi,
                                    queue_TrackingMode,
                                    queue_mcu_intopic,
                                    queue_mcu_outtopic,
                                    ex_holotoraspi,
                                    ex_TrackingMode,
                                    ex_mcu,
                                    rk_holotoraspi,
                                    rk_TrackingMode,
                                    rk_mcu_intopic);


        }

        protected override void OnUpdate(float timeStep)
        //fungsi yang meng-update seluruh nilai pada UI
        {
            base.OnUpdate(timeStep); //set awal

            bool kirim = false;


            timesec = timesec + timeStep;

            //Debug.WriteLine("Waktu on update = " + timesec);

            if (timesec > 0.2)
            {
                kirim = true;
                timesec = 0;
            }

            var dataRMQ = ""; //inisiasi nilai nol

            var dataServer = "";

            if (rmq.GetMessage_Raspi() != "") //jika message baru masuk
            {
                //menggunakan error handler
                try
                {
                    var dataJson = JsonConvert.DeserializeObject<JSON_raspitoholo>(rmq.GetMessage_Raspi());
                    dataRMQ = rmq.GetMessage_Raspi();
                    //ui.updateUI(dataJson.x.ToString(), dataJson.y.ToString(), dataJson.z.ToString(), rmq.GetMessage());
                    headingText.Value = "Heading: " + dataJson.head;
                    positionText.Value = "Latitude : " + (dataJson.latitude) + "; Longitude : " + (dataJson.longitude);
                }
                catch (JsonSerializationException jsonSerializerEx)
                {
                    Debug.WriteLine("Json Serialization Exception : " + jsonSerializerEx.Message.ToString());
                }
                catch (JsonReaderException jsonReaderEx)
                {
                    Debug.WriteLine("Json Reader Exception : " + jsonReaderEx.Message.ToString());

                }
                catch (JsonException jsonEx)
                {
                    Debug.WriteLine("Json Exception : " + jsonEx.Message.ToString());
                }
                catch (Exception ex)
                {
                    Debug.WriteLine("Exception : " + ex.Message.ToString());
                }
            }


            //Menerima data dari server
            if (rmq.GetMessage_Server() != "") //jika message baru masuk
            {
                //menggunakan error handler
                try
                {
                    List<JSON_servertoholo> listdataJson = JsonConvert.DeserializeObject<List<JSON_servertoholo>>(rmq.GetMessage_Server());

                    if ((Int32.Parse(listdataJson[0].tlx) == 0) &&
                        (Int32.Parse(listdataJson[0].tly) == 0) &&
                        (Int32.Parse(listdataJson[0].brx) == 0) &&
                        (Int32.Parse(listdataJson[0].bry) == 0))
                    {
                        sum_box = 0;
                    }
                    else
                    {
                        sum_box = listdataJson.Count;

                        tlx.Clear(); tly.Clear(); brx.Clear(); bry.Clear();
                        for (int i = 0; i <= sum_box - 1; i++)
                        {
                            //Debug.WriteLine("tlx JSON = " + (listdataJson[i].tlx));
                            //Debug.WriteLine("tly JSON = " + (listdataJson[i].tly));
                            //Debug.WriteLine("brx JSON = " + (listdataJson[i].brx));
                            //Debug.WriteLine("bry JSON = " + (listdataJson[i].bry));

                            tlx.Add((Int32.Parse(listdataJson[i].tlx) * 1280 / 720) - 640);
                            tly.Add((Int32.Parse(listdataJson[i].tly) * 720 / 480) - 360);
                            brx.Add((Int32.Parse(listdataJson[i].brx) * 1280 / 720) - 640);
                            bry.Add((Int32.Parse(listdataJson[i].bry) * 720 / 480) - 360);

                            //bounding_box[i].titiktengah.x = (bounding_box[i].tlx + bounding_box[i].brx) / 2;
                            //bounding_box[i].titiktengah.y = (bounding_box[i].tly + bounding_box[i].bry) / 2;

                            //Debug.WriteLine("tlx = {0}", tlx[i]);
                            //Debug.WriteLine("tly = {0}", tly[i]);
                            //Debug.WriteLine("brx = {0}", brx[i]);
                            //Debug.WriteLine("bry = {0}", bry[i]);
                        }
                    }


                    dataServer = rmq.GetMessage_Server();

                    //Debug.WriteLine("jumlah kotak = {0}", sum_box);
                    //Debug.WriteLine("message rmq = {0}", dataServer);
                    //Debug.WriteLine("tlx JSON = " + (listdataJson[0].tlx));
                    //Debug.WriteLine(dataServer);


                }
                catch (JsonSerializationException jsonSerializerEx)
                {
                    Debug.WriteLine("Json Serialization Exception : " + jsonSerializerEx.Message.ToString());
                }
                catch (JsonReaderException jsonReaderEx)
                {
                    Debug.WriteLine("Json Reader Exception : " + jsonReaderEx.Message.ToString());

                }
                catch (JsonException jsonEx)
                {
                    Debug.WriteLine("Json Exception : " + jsonEx.Message.ToString());
                }
                catch (Exception ex)
                {
                    Debug.WriteLine("Exception : " + ex.Message.ToString());
                }
            }


            //Debug.WriteLine("Sum box = {0}", sum_box);
            //Debug.WriteLine("Sum box lama = {0}", sum_box_lama);
            bool ada_isi;


            if (sum_box != 0)
                ada_isi = true;
            else
                ada_isi = false;

            if ((ada_isi_awal == false) && (ada_isi == false))
            {

                //Debug.WriteLine("Gada kotak, do nothing");
                // do nothing;
            }
            else if ((ada_isi_awal == false) && (ada_isi == true))
            {
                for (int i = 0; i <= sum_box - 1; i++)
                {
                    bounding_box.Add(new KotakTarget(tlx[i], tly[i], brx[i], bry[i],
                                                        UI.Root, Context, ResourceCache.GetTexture2D(@"2D/redbox.png")));
                    //Debug.WriteLine(bounding_box[i] is KotakTarget);

                    //bounding_box[i].Delete();

                    //bounding_box[i] = new KotakTarget(bounding_box[i].tlx, bounding_box[i].tly, bounding_box[i].brx, bounding_box[i].bry, UI.Root, Context, ResourceCache.GetTexture2D(@"2D/redbox.png"));
                }

                Debug.WriteLine("Buat kotak");
            }
            else if ((ada_isi_awal == true) && (ada_isi == false))
            {
                for (int i = 0; i <= sum_box_lama - 1; i++)
                {
                    bounding_box[0].Delete();
                    bounding_box.Remove(bounding_box[0]);
                    //bounding_box.Remove(bounding_box[i]);
                    //bounding_box[i].Delete();
                }

                Debug.WriteLine("Delete kotak");
                //bounding_box.Clear();

            }
            else
            {
                Debug.WriteLine("Jumlah bounding box = {0}", bounding_box.Count);
                for (int i = 0; i <= sum_box_lama - 1; i++)
                {
                    Debug.WriteLine(i);
                    //Debug.WriteLine(bounding_box[i].ToString());
                    bounding_box[0].Delete();
                    bounding_box.Remove(bounding_box[0]);
                    //bounding_box.Remove(bounding_box[i]);
                }

                for (int i = 0; i <= sum_box - 1; i++)
                {
                    bounding_box.Add(new KotakTarget(tlx[i], tly[i], brx[i], bry[i],
                                                        UI.Root, Context, ResourceCache.GetTexture2D(@"2D/redbox.png")));
                }
                /*if (sum_box_lama == sum_box)
                {
                    for (int i = 0; i <= sum_box - 1; i++)
                    {
                        bounding_box[i].UpdateBoundingBox(tlx[i], tly[i], brx[i], bry[i]);
                    }
                }
                else*/
                Debug.WriteLine("Update kotak");

            }

            //Debug.WriteLine("Jumlah bounding box = {0}", bounding_box.Count);

            ada_isi_awal = ada_isi;

            for (int i = 0; i <= sum_box - 1; i++)
            {
                //Debug.WriteLine("TLX = {0}", bounding_box[i].tlx);
                //Debug.WriteLine("TLY = {0}", bounding_box[i].tly);
                //Debug.WriteLine("BRX = {0}", bounding_box[i].brx);
                //Debug.WriteLine("BRY = {0}", bounding_box[i].bry);
            }

            //mengambil nilai yaw dan pitch dari rightcamera hololens
            yawText.Value = (((-1)*(RightCamera.Node.WorldRotation.YawAngle + LeftCamera.Node.WorldRotation.YawAngle) / 2)).ToString();
            pitchText.Value = (((-1) * (RightCamera.Node.WorldRotation.PitchAngle + LeftCamera.Node.WorldRotation.PitchAngle) / 2)).ToString();


            //mengambil nilai posisi dari rightcamera hololens
            positionText.Value = (RightCamera.Node.WorldPosition.X + LeftCamera.Node.WorldPosition.X) / 2 + " X; " +
                               (RightCamera.Node.WorldPosition.Y + LeftCamera.Node.WorldPosition.Y) / 2 + " Y; " +
                               (RightCamera.Node.WorldPosition.Z + LeftCamera.Node.WorldPosition.Z) / 2 + " Z; ";

            //mulai pengerjaan yaw dan pitch yang akan dikirim
            float yawfromBoundingBox;
            float pitchfromBoundingBox;
            string sendjson;

            JSON_holotoraspi account = new JSON_holotoraspi
            {
                yaw = 0,
                pitch = 0,
                fire = tembak,
                indBoxTrackedPlusOne = indBoxTrackedPlusOne
            };

            if (sum_box < indBoxTrackedPlusOne)
            {
                indBoxTrackedPlusOne = 0;
            }

            if (indBoxTrackedPlusOne == 0)
            {
                //Manual mode
                //DEBUG
                account.yaw = float.Parse(yawText.Value);
                account.pitch = float.Parse(pitchText.Value);
                account.fire = tembak;
                account.indBoxTrackedPlusOne = indBoxTrackedPlusOne;
                sendjson = JsonConvert.SerializeObject(account, Formatting.Indented);

                if (kirim == true)
                {
                    rmq.SendMessage(ex_holotoraspi, rk_holotoraspi, sendjson);
                }

                if (tembak == 0)
                {
                    //readyText.Value = "TRACKING";
                    //readyText.SetColor(Color.Green);

                    readyText.Value = "READY";
                    readyText.SetColor(Color.White);
                }

            }
            else
            {
                //Tracking mode

                if (tembak == 0)
                {
                    readyText.Value = ("TRACKING " + indBoxTrackedPlusOne.ToString());
                    readyText.SetColor(Color.Green);
                }

                yawfromBoundingBox = ((float.Parse(yawText.Value) +
                                                        (-1)*Convert.ToSingle(Math.Atan(bounding_box[indBoxTrackedPlusOne - 1].titiktengah.x / 4290.02255) / Math.PI * 180.0)));
                pitchfromBoundingBox = ((float.Parse(pitchText.Value) +
                                                        (-1)*Convert.ToSingle(Math.Atan(bounding_box[indBoxTrackedPlusOne - 1].titiktengah.y / 4309.6764) / Math.PI * 180.0)));
                //yawfromBoundingBox = ((float.Parse(yawText.Value) +
                //                                        Convert.ToSingle(Math.Atan(bounding_box[indBoxTrackedPlusOne - 1].titiktengah.x / 1060.94) / Math.PI * 180.0)));
                //pitchfromBoundingBox = ((float.Parse(pitchText.Value) +
                //                                        Convert.ToSingle(Math.Atan(bounding_box[indBoxTrackedPlusOne - 1].titiktengah.y / 793.6156) / Math.PI * 180.0)));
                //DEBUG
                
                account.yaw = (yawfromBoundingBox);
                account.pitch = (pitchfromBoundingBox);
                account.fire = tembak;
                account.indBoxTrackedPlusOne = indBoxTrackedPlusOne;
                sendjson = JsonConvert.SerializeObject(account, Formatting.Indented);

                if (kirim == true)
                {
                    rmq.SendMessage(ex_holotoraspi, rk_holotoraspi, sendjson);
                    rmq.SendMessage(ex_mcu, rk_mcu_intopic, sendjson);
                }



                
            }

            sum_box_lama = sum_box;

            // await Delay(5f);
            //mengirim objek json ke server rabbitmq


            Debug.WriteLine("delay holo = " + timeStep);

        }

        public override void OnGestureTapped()
        {
            
            List<int> BoxClickedList = new List<int>();

            //readyText.Value = "TRACKING";
            //readyText.SetColor(Color.Green);

            if (indBoxTrackedPlusOne != 0)
            {
                indBoxTrackedPlusOne = 0;
            }
            else
            {

                //int j = 0;
                for (int i = 0; i <= sum_box - 1; i++)
                {
                    if ((bounding_box[i].brx >= 0) && (bounding_box[i].tlx <= 0) && (0 >= bounding_box[i].tly) && (0 <= bounding_box[i].bry))
                    //if(bounding_box.Count > 0)
                    {
                        //BoxClickedList[j] = i + 1;
                        BoxClickedList.Add(i + 1);
                        //j++;

                        //readyText.Value = "TRACKING";
                        //readyText.SetColor(Color.Green);
                        //Debug.WriteLine("Masuk sini!!");
                    }
                }

                int sum_clickedBox = BoxClickedList.Count;
                //Debug.WriteLine("Jumlah : {0}", sum_clickedBox);
                if (sum_clickedBox != 0)
                {
                    Random rnd = new Random();
                    indBoxTrackedPlusOne = BoxClickedList[rnd.Next(0, sum_clickedBox - 1)];
                }
                else
                {
                    indBoxTrackedPlusOne = 0;
                }
            }

            base.OnGestureTapped();

        }

        public override async void OnGestureDoubleTapped()
        //ketika hololens menerima input double-tap
        {
            //Debug.WriteLine("Click happen");
            readyText.Value = "FIRE";
            readyText.SetColor(Color.Red);
            tembak = 1;

            await Delay(1f);
            tembak = 0;
            readyText.Value = "READY";
            readyText.SetColor(Color.White);
            base.OnGestureDoubleTapped();
        }


        void ShowYaw()
        //fungsi menampilkan yaw
        {
            //Menampilkan yaw
            yawText = new Text(Context);
            yawText.Value = "0";
            yawText.HorizontalAlignment = HorizontalAlignment.Center;
            yawText.VerticalAlignment = VerticalAlignment.Top;
            yawText.Position = new IntVector2(0, 100);
            yawText.SetColor(Color.White);
            yawText.SetFont(CoreAssets.Fonts.AnonymousPro, 30);
            UI.Root.AddChild(yawText);
        }

        void ShowPitch()
        //fungsi menampilkan pitch
        {
            //Menampilkan pitch
            pitchText = new Text(Context);
            pitchText.Value = "0.0";
            pitchText.HorizontalAlignment = HorizontalAlignment.Left;
            pitchText.VerticalAlignment = VerticalAlignment.Center;
            pitchText.Position = new IntVector2(300, 0);
            pitchText.SetColor(Color.White);
            pitchText.SetFont(CoreAssets.Fonts.AnonymousPro, 30);
            UI.Root.AddChild(pitchText);
        }

        void ShowReady()
        //fungsi menampilkan keterangan ready/fire
        {
            readyText = new Text(Context);
            readyText.Value = "READY";
            readyText.HorizontalAlignment = HorizontalAlignment.Right;
            readyText.VerticalAlignment = VerticalAlignment.Top;
            readyText.Position = new IntVector2(-200, 150);
            readyText.SetColor(Color.White);
            readyText.SetFont(CoreAssets.Fonts.AnonymousPro, 30);
            UI.Root.AddChild(readyText);
        }

        void ShowHeading()
        //fungsi menampilkan sudut heading
        {
            headingText = new Text(Context);
            headingText.Value = "Heading :" + rmq.GetMessage_Raspi();
            headingText.HorizontalAlignment = HorizontalAlignment.Right;
            headingText.VerticalAlignment = VerticalAlignment.Bottom;
            headingText.SetColor(Color.White);
            headingText.Position = new IntVector2(-200, -200);
            headingText.SetFont(CoreAssets.Fonts.AnonymousPro, 30);
            UI.Root.AddChild(headingText);
        }

        void ShowPosition()
        //fungsi menampilkan posisi turret
        {
            positionText = new Text();
            positionText.Value = "0" + " X; " + "0" + " Y; " + "0" + " Z; ";

            positionText.HorizontalAlignment = HorizontalAlignment.Right;
            positionText.VerticalAlignment = VerticalAlignment.Top;
            positionText.Position = new IntVector2(-100, 200); // 0, 50
            positionText.SetColor(Color.White);
            positionText.SetFont(CoreAssets.Fonts.AnonymousPro, 30);
            UI.Root.AddChild(positionText);
        }

        void ShowCrosshair()
        //fungsi menampilkan crosshair
        {
            //buat crosshair
            crosshair = new Sprite(Context);
            crosshair.Texture = ResourceCache.GetTexture2D(@"2D/crosshair1.png");
            crosshair.SetAlignment(HorizontalAlignment.Center, VerticalAlignment.Center);
            crosshair.Position = new IntVector2(-50, -50);
            crosshair.SetSize(100, 100);
            UI.Root.AddChild(crosshair);
        }

    }
}

