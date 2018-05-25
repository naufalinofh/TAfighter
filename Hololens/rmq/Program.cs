using RabbitMQ.Client;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Publisher_cmd
{
    class Program
    {
        static void Main(string[] args)
        {
            RMQ rmq = new RMQ();
            Console.WriteLine("Tekan tombol apapun untuk inisialisasi RMQ parameters.");
            Console.ReadKey();
            rmq.InitRMQConnection(); // inisialisasi parameter (secara default) untuk koneksi ke server RMQ
            Console.WriteLine("Tekan tombol apapun untuk membuka koneksi ke RMQ."); Console.ReadKey();
            rmq.CreateRMQConnection(); // memulai koneksi dengan RMQ
            Console.Write("Masukkan nama queue channel untuk mengirim pesan melalui RMQ.\n>> ");
            string queue_name = Console.ReadLine();
            rmq.CreateRMQChannel(queue_name);
            Console.Write("Masukkan pesan yang akan dikirim atau 'exit' to close.\n>>");
            Console.Write(">> tujuan: ");
            string tujuan = Console.ReadLine();
            Console.Write(">> pesan: ");
            string inputMsg = Console.ReadLine();
            while (inputMsg != "exit")
            {
                rmq.SendMessage(tujuan, inputMsg);
                Console.Write(">> tujuan: ");
                tujuan = Console.ReadLine();
                Console.Write(">> pesan: ");
                inputMsg = Console.ReadLine();
            }
            rmq.Disconnect();
        }
    }

    class RMQ
    {
        public ConnectionFactory connectionFactory;
        public IConnection connection;
        public IModel channel;
        public void InitRMQConnection(string host = "localhost", int port = 5672, string user = "guest",
        string pass = "guest")
        {
            connectionFactory = new ConnectionFactory();
            connectionFactory.HostName = host;
            //connectionFactory.Port = port;
            connectionFactory.UserName = user;
            connectionFactory.Password = pass;
        }
        public void CreateRMQConnection()
        {
            connection = connectionFactory.CreateConnection();
            Console.WriteLine("Koneksi " + (connection.IsOpen ? "Berhasil!" : "Gagal!"));
        }
        public void CreateRMQChannel(string queue_name, string routingKey = "", string exchange_name =
        "")
        {
            if (connection.IsOpen)
            {
                channel = connection.CreateModel();
                Console.WriteLine("Channel " + (channel.IsOpen ? "Berhasil!" : "Gagal!"));
            }
            if (channel.IsOpen)
            {
                channel.QueueDeclare(queue: queue_name,
                durable: true,
                exclusive: false,
                autoDelete: false,
                arguments: null);
                Console.WriteLine("Queue telah dideklarasikan..");
            }
        }
        public void SendMessage(string tujuan, string msg = "send")
        {
            byte[] responseBytes = Encoding.UTF8.GetBytes(msg);// konversi pesan dalam bentuk string menjadi byte
            channel.BasicPublish(exchange: "",
            routingKey: tujuan,
            basicProperties: null,
            body: responseBytes);
            Console.WriteLine("Pesan: '" + msg + "' telah dikirim.");
        }
        public void Disconnect()
        {
            channel.Close();
            channel = null;
            Console.WriteLine("Channel ditutup!");
            if (connection.IsOpen)
            {
                connection.Close();
            }
            Console.WriteLine("Koneksi diputus!");
            connection.Dispose();
            connection = null;
        }
    }
}

