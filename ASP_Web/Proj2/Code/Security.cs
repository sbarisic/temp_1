using Microsoft.AspNetCore.Components.Authorization;
using Microsoft.AspNetCore.Components.Server;
using Microsoft.AspNetCore.Identity;
using Microsoft.Extensions.Options;
using System.Security.Claims;
using System.Security.Cryptography;
using Microsoft.AspNetCore.Authentication.Cookies;
using Microsoft.AspNetCore.Components;
using Proj2.Database;
using Microsoft.EntityFrameworkCore.Scaffolding.Metadata;

namespace Proj2.Code {
	public class PasswdManager {
		public PasswdManager() {
			//PasswordHasher<string> passwordHasher = new PasswordHasher<string>();


		}
	}

	/*public class UserIdentity {
		public UserIdentity() {
		}
	}*/

	class AuthStateProvider : AuthenticationStateProvider, IHostEnvironmentAuthenticationStateProvider {
		AuthenticationState AuthState;

		public override Task<AuthenticationState> GetAuthenticationStateAsync() {
			if (AuthState == null)
				Logout();

			return Task.FromResult(AuthState);
		}

		public void SetAuthenticationState(Task<AuthenticationState> authenticationStateTask) {
			if (authenticationStateTask == null) {
				Logout();
			} else {
				AuthState = authenticationStateTask.Result;
			}

			NotifyAuthenticationStateChanged(GetAuthenticationStateAsync());
		}

		public void Logout() {
			ClaimsPrincipal Princ = new ClaimsPrincipal();
			AuthState = new AuthenticationState(Princ);
		}

		public bool Login(string Username, string Password, out AuthenticationState AuthState) {
			DatabaseContext Db = DatabaseService.Instance.Database;
			AuthState = null;

			DbUser DbUsr = Db.Users.Where(Usr => Usr.Username == Username).FirstOrDefault();
			if (DbUsr == null)
				return false;


			List<Claim> Claims = new List<Claim>();
			Claims.Add(new Claim("Username", Username));

			ClaimsIdentity Ident = new ClaimsIdentity(Claims, CookieAuthenticationDefaults.AuthenticationScheme);
			ClaimsPrincipal Princ = new ClaimsPrincipal(Ident);

			AuthState = new AuthenticationState(Princ);
			return true;
		}

		public DbUser GetDbUser() {
			if (AuthState == null || AuthState.User == null)
				return null;

			DatabaseContext Db = DatabaseService.Instance.Database;

			Claim UsernameClaim = AuthState.User.FindFirst("Username");
			if (UsernameClaim == null)
				return null;

			DbUser DbUsr = Db.Users.Where(Usr => Usr.Username == UsernameClaim.Value).FirstOrDefault();
			return DbUsr;
		}
	}
}
