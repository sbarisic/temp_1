using Microsoft.AspNetCore.SignalR;
using System.Diagnostics;

namespace Proj2.Hubs {
    public class ComHub : Hub {
        public async Task SendMessage(string user, string message) {
            await Clients.All.SendAsync("ReceiveMessage", user, message);
        }

        public async void DebugLog(string Msg) {
            Debug.WriteLine("DEBUGLOG: " + Msg);
        }
    }
}
