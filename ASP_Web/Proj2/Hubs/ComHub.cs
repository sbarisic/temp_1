using Microsoft.AspNetCore.SignalR;
using Proj2.Code;
using System.Diagnostics;

namespace Proj2.Hubs {
	public class ComHub : Hub {
		public async Task SendMessage(string user, string message) {
			await Clients.All.SendAsync("ReceiveMessage", user, message);
		}

		public async Task SendUpdateItemData(ItemData Item) {
			await Clients.All.SendAsync("ReceiveUpdateItemData", Item);
		}

		public async void DebugLog(string Msg) {
			await Task.Run(() => {
				Debug.WriteLine("DEBUGLOG: " + Msg);
			});
		}
	}
}
