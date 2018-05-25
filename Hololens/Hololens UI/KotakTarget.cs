using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Urho;
using Urho.Gui;
using Urho.Resources;


namespace Hololens_UI
{
    class KotakTarget
    {
        public int tlx, tly, brx, bry;

         

        public struct koor
        {
            public int x, y;

            public koor(int p1, int p2)
            {
                x = p1;
                y = p2;
            }
        }

        public koor titiktengah;

        protected UIElement root;
        protected Context context;
        protected Sprite bounding_box;
        protected koor topleft, topright, bottomleft, bottomright, boxsize;

        //public ListKotakTarget ()
        //{

        //}

        public KotakTarget(int topleft_x, int topleft_y, int bottomright_x, int bottomright_y, UIElement ui_root, Context con, Texture kotak)
        {
            //Debug.WriteLine("Mulai ");
            root = ui_root;
            context = con;
            topleft.x = topleft_x; topleft.y = topleft_y;
            topright.x = bottomright_x; topright.y = topleft_y;
            bottomright.x = bottomright_x; bottomright.y = bottomright_y;
            bottomleft.x = topleft_x; bottomleft.y = bottomright_y;

            tlx = topleft_x; tly = topleft_y; brx = bottomright_x; bry = bottomright_y;

            titiktengah.x = ((tlx + brx)/2); titiktengah.y = ((tly + bry) / 2);

            bounding_box = new Sprite(context);
            bounding_box.Texture = kotak;
            bounding_box.SetAlignment(HorizontalAlignment.Center, VerticalAlignment.Center);
            bounding_box.Position = new IntVector2(topleft.x, topleft.y);
            bounding_box.SetSize(Math.Abs(bottomright.x - topleft.x), Math.Abs(topleft.y - bottomright.y));
            root.AddChild(bounding_box);
            //Debug.WriteLine("Show bounding box ok ");
        }

        public void UpdateBoundingBox(int topleft_x, int topleft_y, int bottomright_x, int bottomright_y)
        {
            topleft.x = topleft_x; topleft.y = topleft_y;
            topright.x = bottomright_x; topright.y = topleft_y;
            bottomright.x = bottomright_x; bottomright.y = bottomright_y;
            bottomleft.x = topleft_x; bottomleft.y = bottomright_y;

            tlx = topleft_x; tly = topleft_y; brx = bottomright_x; bry = bottomright_y;

            titiktengah.x = ((tlx + brx) / 2); titiktengah.y = ((tly + bry) / 2);

            bounding_box.Position = new IntVector2(topleft.x, topleft.y);
            bounding_box.SetSize(Math.Abs(bottomright.x - topleft.x), Math.Abs(topleft.y - bottomright.y));
        }

        /*~KotakTarget()
        {
            bounding_box.Remove();
        }*/

        public void Delete()
        {
            bounding_box.Remove();
        }
    }
}
