using System;
using System.Threading;
using System.Net.Sockets;
using System.Text;
using System.Net;
using System.Linq;
using System.Linq.Dynamic;
using System.Reflection;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;

class Message
{
	public string Sender;
	public string Receiver;
	public string Text;
	public DateTime SentAt;
	public int Id;
	public IPAddress ip4;

	public Message(string sender, string receiver, string text, DateTime sentAt, int id, IPAddress ip4)
	{
		this.Sender = sender;
		this.Receiver = receiver;
		this.Text = text;
		this.SentAt = sentAt;
		this.Id = id;
		this.ip4 = ip4;
	}
}



namespace linq_to_t
{
	class Program
	{
		static void Main(string[] args)
		{
			System.Net.IPAddress ipaddress = System.Net.IPAddress.Parse("0.0.0.0");
			TcpListener serverSocket = new TcpListener(ipaddress, 8888);
			TcpClient clientSocket = default(TcpClient);
			int counter = 0;

			serverSocket.Start();
			Console.WriteLine(" >> " + "Server Started");

			counter = 0;

			while (true)
			{
				counter += 1;
				clientSocket = serverSocket.AcceptTcpClient();
				Console.WriteLine(" >> " + "Client No:" + Convert.ToString(counter) + " started!");
				handleClinet client = new handleClinet();
				client.startClient(clientSocket, Convert.ToString(counter));
			}

			clientSocket.Close();
			serverSocket.Stop();
			Console.WriteLine(" >> " + "exit");
			Console.ReadLine();
		}
	}

	//Class to handle each client request separatly
	public class handleClinet
	{
		TcpClient clientSocket;
		string clNo;
		public void startClient(TcpClient inClientSocket, string clineNo)
		{
			this.clientSocket = inClientSocket;
			this.clNo = clineNo;
			Thread ctThread = new Thread(doChat);
			ctThread.Start();
		}

		private void doChat()
		{
			byte[] bytesFrom = new byte[10025];
			Byte[] sendBytes = null;
			string serverResponse = null;
			string dataFromClient;

			var messages = new Message[] {
				new Message("Hilary", "Donald", "You are going down!", DateTime.Now, 1, IPAddress.Parse("127.0.0.1")),
				new Message("Donald", "Hilary", "Oh I don't think so. My polls are HUUUUGGEEE.", DateTime.Now, 2, IPAddress.Parse("127.0.0.1")),
				new Message("Hilary", "Donald", "Nice chat!", DateTime.Now, 3, IPAddress.Parse("127.0.0.1")),
				new Message("Donald", "Hilary", "Bye loser.", DateTime.Now, 4, IPAddress.Parse("127.0.0.1")),
				new Message("Hilary", "Donald", "No, you are the loser.", DateTime.Now, 5, IPAddress.Parse("127.0.0.1"))
			};

			try
			{
				NetworkStream networkStream = clientSocket.GetStream();
				networkStream.Read(bytesFrom, 0, 2048);
				dataFromClient = System.Text.Encoding.ASCII.GetString(bytesFrom);
				dataFromClient = dataFromClient.Replace("\n", "").Trim('\0');
				Console.WriteLine(" >> " + "From client-" + clNo + dataFromClient);

				var filteredMessages = messages.Where("Text = " + dataFromClient);

				foreach (var message in filteredMessages)
				{
					sendBytes = Encoding.ASCII.GetBytes(message.Sender + " -> " + message.Receiver + ": " + message.Text);
					networkStream.Write(sendBytes, 0, sendBytes.Length);
					networkStream.Flush();
				}
				Console.WriteLine(" >> " + serverResponse);
			}
			catch (Exception ex)
			{
				Console.WriteLine(" >> " + ex.ToString());
			}
			clientSocket.Close();
		}
	}
}