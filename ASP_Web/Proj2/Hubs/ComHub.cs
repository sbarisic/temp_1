using Microsoft.AspNetCore.SignalR;
using Proj2.Code;
using Proj2.Database;
using System.Diagnostics;

namespace Proj2.Hubs {
	public class ComHub : Hub {
		public async Task SendStateHasChanged() {
			await Clients.All.SendAsync("OnStateHasChanged");
		}
	}

	public class ComHubService {
		public static ComHubService Instance {
			get; private set;
		}

		IHubContext<ComHub> HubContext;

		public ComHubService(IHubContext<ComHub> HubContext) {
			this.HubContext = HubContext;
			Instance = this;
		}

		public void OnStateHasChanged(DbDeviceAPIKey APIKey) {


			HubContext.Clients.All.SendAsync("OnStateHasChanged").Wait();
		}

		public void JsonDataReceived(DbJsonLog JsonLog) {
		
		}
	}
}
