using Microsoft.AspNetCore.SignalR;

namespace Proj2.Hubs {
    public class ComHub : Hub {
        public async Task SendMessage(string user, string message) {
            await Clients.All.SendAsync("ReceiveMessage", user, message);
        }
    }
}
