using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using RabbitMQ.Client;
using RabbitMQ.Client.Events;

namespace Hololens_UI
{
    class RMQ
    {
        public ConnectionFactory connectionFactory;
        public IConnection connection;
        public IModel channel;

        string message_raspi = "";
        string message_server = "";
        byte[] body;

        public void InitRMQConnection(string host = "167.205.7.226", int port = 5672, string user = "turret.tank030",
        string pass = "TA171801030", string vhost = "/turret")
        //public void InitRMQConnection(string host = "192.168.43.37", int port = 5672, string user = "anharaf",
        //string pass = "anhar1234", string vhost = "/")
        //public void InitRMQConnection(string host = "192.168.43.49", int port = 5672, string user = "rangga",
        //string pass = "1234", string vhost = "/")
        {
            connectionFactory = new ConnectionFactory();
            connectionFactory.HostName = host;
            connectionFactory.Port = port;
            connectionFactory.UserName = user;
            connectionFactory.Password = pass;
            connectionFactory.VirtualHost = vhost;
        }
        public void CreateRMQConnection()
        {
            connection = connectionFactory.CreateConnection();
        }
        public void CreateRMQChannel(string queue_consume_raspi,
                                        string queue_consume_server,
                                        string queue_publish,
                                        string queue_publish_trackmode,
                                        string queue_mcu_intopic,
                                        string queue_mcu_outtopic,
                                        string exchange_name_publish,
                                        string exchange_name_publish_trackmode,
                                        string ex_mcu,
                                        string routing_key_publish,
                                        string routing_key_publish_trackmode,
                                        string rk_mcu_intopic)
        {
            if (connection.IsOpen)
            {
                channel = connection.CreateModel();
            }
            if (channel.IsOpen)
            {
                Dictionary<string, object> args = new Dictionary<string, object>()
                    {
                        { "x-max-length", 2 }
                    };

                //channel.QueueDelete(queue_consume_raspi);
                //channel.QueueDelete(queue_consume_server);
                //channel.QueueDelete(queue_publish);
                //channel.QueueDelete(queue_mcu_intopic);
                //channel.QueueDelete(queue_mcu_outtopic);

                channel.ExchangeDeclare(exchange: exchange_name_publish,
                                        type: "topic",
                                        durable: false);

                channel.ExchangeDeclare(exchange: exchange_name_publish_trackmode,
                                                        type: "topic",
                                                        durable: false);

                channel.ExchangeDeclare(exchange: ex_mcu,
                                        type: "topic",
                                        durable: true);

                channel.QueueDeclare(queue: queue_consume_raspi,
                                     durable: false,
                                     exclusive: false,
                                     autoDelete: false,
                                     arguments: args);

                channel.QueueDeclare(queue: queue_consume_server,
                                     durable: false,
                                     exclusive: false,
                                     autoDelete: false,
                                     arguments: args);

                channel.QueueDeclare(queue: queue_publish,
                                     durable: false,
                                     exclusive: false,
                                     autoDelete: false,
                                     arguments: args);

                channel.QueueDeclare(queue: queue_publish_trackmode,
                                     durable: false,
                                     exclusive: false,
                                     autoDelete: false,
                                     arguments: args);

                channel.QueueDeclare(queue: queue_mcu_intopic,
                                     durable: false,
                                     exclusive: false,
                                     autoDelete: false,
                                     arguments: args);

                channel.QueueDeclare(queue: queue_mcu_outtopic,
                                     durable: false,
                                     exclusive: false,
                                     autoDelete: false,
                                     arguments: args);

                channel.QueueBind(queue: queue_publish,
                                  exchange: exchange_name_publish,
                                  routingKey: routing_key_publish);

                channel.QueueBind(queue: queue_publish_trackmode,
                                  exchange: exchange_name_publish_trackmode,
                                  routingKey: routing_key_publish_trackmode);

                channel.QueueBind(queue: queue_mcu_intopic,
                                  exchange: ex_mcu,
                                  routingKey: rk_mcu_intopic);

                channel.QueuePurge(queue_consume_raspi);
                channel.QueuePurge(queue_consume_server);
                channel.QueuePurge(queue_publish);
                channel.QueuePurge(queue_publish_trackmode);
                channel.QueuePurge(queue_mcu_intopic); 
                channel.QueuePurge(queue_mcu_outtopic);


                var consumer_raspi = new EventingBasicConsumer(channel);
                consumer_raspi.Received += (model, ea) =>
                {
                    body = ea.Body;
                    message_raspi = Encoding.UTF8.GetString(body);
                    //Debug.WriteLine(" [x] Pesan diterima: {0}", message_raspi);
                };

                channel.BasicConsume(queue: queue_consume_raspi,
                                     noAck: true,
                                     consumer: consumer_raspi);

                var consumer_server = new EventingBasicConsumer(channel);
                consumer_server.Received += (model, ea) =>
                {
                    body = ea.Body;
                    message_server = Encoding.UTF8.GetString(body);
                    //Debug.WriteLine(" [x] Pesan diterima: {0}", message_server);
                };

                channel.BasicConsume(queue: queue_consume_server,
                                     noAck: true,
                                     consumer: consumer_server);

            }
        }

        public void SendMessage(string exchange_name, string routing_key, string msg = "send")
        {
            byte[] responseBytes = Encoding.UTF8.GetBytes(msg);
            channel.BasicPublish(exchange: exchange_name,
                                routingKey: routing_key,
                                basicProperties: null,
                                body: responseBytes);
        }

        public string GetMessage_Raspi()
        {
            return message_raspi;
        }

        public string GetMessage_Server()
        {
            return message_server;
        }

        public void Disconnect()
        {
            channel.Close();
            channel = null;

            if (connection.IsOpen)
            {
                connection.Close();
            }

            connection.Dispose();
            connection = null;
        }

        public void Delete(string queue)
        {
            channel.QueueDelete(queue);
        }
    }
}
