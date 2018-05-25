using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Hololens_UI
{
    public class JSON_raspitoholo
    {
        //data x y z dari sensor gps arduino

        public string head { get; set; }
        public string sensor1 { get; set; }
        //data x y z dari sensor gps arduino
        public string sensor2 { get; set; }
        public string latitude { get; set; }
        public string longitude { get; set; }
        //data heading dari sensor compass arduino
        //public double head { get; set; }
    }

    public class JSON_holotoraspi
    {
        //data yaw dan pitch dari hololens
        //DEBUG
        public float yaw { get; set; }
        public float pitch { get; set; }
        public int fire { get; set; }
        public int indBoxTrackedPlusOne { get; set; }
    }

    public class Rootobject
    {
        public JSON_servertoholo[] Property1 { get; set; }
    }

    public class JSON_servertoholo
    {
        public string tlx { get; set; }
        public string tly { get; set; }
        public string brx { get; set; }
        public string bry { get; set; }
    }
}
