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
using Microsoft.AspNetCore.Cryptography.KeyDerivation;

namespace Proj2.Code {
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
			AuthState = null;

			using (DatabaseContext Db = new DatabaseContext()) {
				DbUser DbUsr = Db.Users.Where(Usr => Usr.Username == Username).FirstOrDefault();

				if (DbUsr == null)
					return false;


				using (PasswdManager PassMgr = new PasswdManager()) {
					string SaltedHash = PassMgr.GenerateHash(Password, DbUsr.Salt);

					if (SaltedHash != DbUsr.Hash)
						return false;
				}

				List<Claim> Claims = new List<Claim>();
				Claims.Add(new Claim("Username", Username));
				Claims.Add(new Claim("UserID", DbUsr.ID.ToString()));

				ClaimsIdentity Ident = new ClaimsIdentity(Claims, CookieAuthenticationDefaults.AuthenticationScheme);
				ClaimsPrincipal Princ = new ClaimsPrincipal(Ident);

				AuthState = new AuthenticationState(Princ);
				return true;
			}
		}

		public DbUser GetDbUser() {
			if (AuthState == null || AuthState.User == null)
				return null;

			Claim UserIDClaim = AuthState.User.FindFirst("UserID");

			if (UserIDClaim == null)
				return null;

			int UserID = int.Parse(UserIDClaim.Value);

			using (DatabaseContext Db = new DatabaseContext()) {
				DbUser DbUsr = Db.Users.Where(Usr => Usr.ID == UserID).FirstOrDefault();
				return DbUsr;
			}
		}
	}
}
