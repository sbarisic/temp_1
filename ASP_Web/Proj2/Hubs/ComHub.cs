using Microsoft.AspNetCore.SignalR;
using Proj2.Code;
using System.Diagnostics;

namespace Proj2.Hubs {
	public class ComHub : Hub {
		public async Task SendStateHasChanged() {
			await Clients.All.SendAsync("OnStateHasChanged");
		}
	}
}
