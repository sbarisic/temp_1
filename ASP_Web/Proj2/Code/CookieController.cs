using Microsoft.AspNetCore.Authentication;
using Microsoft.AspNetCore.Components.Authorization;
using Microsoft.AspNetCore.Mvc;
using Proj2.Database;
using System.Security.Claims;

namespace Proj2.Code {
	[Route("/[controller]")]
	[ApiController]
	[RequireHttps]
	public class CookieController : ControllerBase {

		/*[HttpPost("/deviceaccess")]
		public JsonResult Post([FromBody] DeviceAccessAPI API) {
			if (API == null || API.APIVersion == 0)
				return new JsonResult(new DeviceAccessResponseAPI(DeviceAccessStatus.Invalid));

			if (string.IsNullOrEmpty(API.APIKey))
				return new JsonResult(new DeviceAccessResponseAPI(DeviceAccessStatus.Forbidden));

			using (DatabaseContext DbCtx = new DatabaseContext()) {
				DbDeviceAPIKey DbAPIKey = DbCtx.APIKeys.Where(AT => AT.APIKey == API.APIKey).FirstOrDefault();

				if (DbAPIKey == null || DbAPIKey.APIKey != API.APIKey)
					return new JsonResult(new DeviceAccessResponseAPI(DeviceAccessStatus.Forbidden));


				//DbCtx.Database.

				Console.WriteLine("Hello Database World!");

			}

			return new JsonResult(new DeviceAccessResponseAPI(DeviceAccessStatus.OK));
		}*/

		[HttpPost("/cookie/login")]
		public async Task<ActionResult> Login([FromForm] string username, [FromForm] string password) {
			if (AuthStateProvider.Login(username, password, out AuthenticationState AuthState)) {
				await HttpContext.SignInAsync(AuthState.User);
				return Redirect("/");
			}

			return Redirect("/login?invalid=1");

			//ClaimsIdentity claimsIdentity = new ClaimsIdentity(new List<Claim> { new Claim(ClaimTypes.NameIdentifier, name) }, "auth");
			//ClaimsPrincipal claims = new ClaimsPrincipal(claimsIdentity);


		}

		[HttpGet("/cookie/logout")]
		public async Task<ActionResult> Logout() {
			//ClaimsIdentity claimsIdentity = new ClaimsIdentity(new List<Claim> { new Claim(ClaimTypes.NameIdentifier, name) }, "auth");
			//ClaimsPrincipal claims = new ClaimsPrincipal(claimsIdentity);

			//await HttpContext.SignInAsync(claims);

			await HttpContext.SignOutAsync();
			return Redirect("/");
		}
	}
}
