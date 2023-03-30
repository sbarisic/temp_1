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
		[HttpPost("/cookie/login")]
		public async Task<ActionResult> Login([FromForm] string username, [FromForm] string password) {
			if (AuthStateProvider.Login(username, password, out AuthenticationState AuthState)) {
				await HttpContext.SignInAsync(AuthState.User);
				return Redirect("/");
			}

			return Redirect("/login?invalid=1");
		}

		[HttpGet("/cookie/logout")]
		public async Task<ActionResult> Logout() {
			await HttpContext.SignOutAsync();
			return Redirect("/");
		}
	}
}
