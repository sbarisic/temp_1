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
using Microsoft.JSInterop;
using System.Text.Json;
using System.Text.Json.Serialization;
using Microsoft.EntityFrameworkCore.Query.Internal;
using System.Reflection.Metadata.Ecma335;
using System.ComponentModel;

namespace Proj2.Code {
	class SessionStorageUser {
		public string Username {
			get; set;
		}

		public int UserID {
			get; set;
		}

		public SessionStorageUser() {
		}

		public SessionStorageUser(string Username, int UserID) {
			this.Username = Username;
			this.UserID = UserID;
		}
	}

	class AuthStateProvider : AuthenticationStateProvider, IHostEnvironmentAuthenticationStateProvider {
		IJSRuntime JSRun;
		//IUserService userService;
		AuthenticationState AuthState;

		public AuthStateProvider(IJSRuntime JSRun) {
			this.JSRun = JSRun;
		}

		public override Task<AuthenticationState> GetAuthenticationStateAsync() {
			if (AuthState == null)
				Logout();

			return Task.FromResult(AuthState);
		}

		public AuthenticationState GetAuthenticationState() {
			return AuthState;
		}


		public void SetAuthenticationState(Task<AuthenticationState> authenticationStateTask) {
			if (authenticationStateTask == null) {
				Logout();
			} else {
				AuthState = authenticationStateTask.Result;
			}

			NotifyAuthenticationStateChanged(GetAuthenticationStateAsync());
		}

		public async void SaveSessionUser(SessionStorageUser SessionUser) {
			string SessionUserJSON = "";

			if (SessionUser != null)
				SessionUserJSON = JsonSerializer.Serialize(SessionUser);

			await JSRun.InvokeVoidAsync("sessionStorage.setItem", "sessionUser", SessionUserJSON);
		}

		public async Task<SessionStorageUser> LoadSessionUser() {
			string SessionUserJSON = await JSRun.InvokeAsync<string>("sessionStorage.getItem", "sessionUser");

			if (string.IsNullOrEmpty(SessionUserJSON))
				return null;

			return JsonSerializer.Deserialize<SessionStorageUser>(SessionUserJSON);
		}

		public void Logout() {
			ClaimsPrincipal Princ = new ClaimsPrincipal();
			AuthState = new AuthenticationState(Princ);
		}

		public bool Login(string Username, string Password, out AuthenticationState AuthState, out SessionStorageUser SessionUser) {
			AuthState = null;
			SessionUser = null;

			using (DatabaseContext Db = new DatabaseContext()) {
				DbUser DbUsr = Db.Users.Where(Usr => Usr.Username == Username).FirstOrDefault();

				if (DbUsr == null)
					return false;


				using (PasswdManager PassMgr = new PasswdManager()) {
					string SaltedHash = PassMgr.GenerateHash(Password, DbUsr.Salt);

					if (SaltedHash != DbUsr.Hash)
						return false;
				}

				AuthState = CreateAuthState(DbUsr.Username, DbUsr.ID);
				SessionUser = new SessionStorageUser(DbUsr.Username, DbUsr.ID);
				return true;
			}
		}

		public async Task<bool> TryLoginFromSession() {
			AuthenticationState AuthState = await GetAuthenticationStateAsync();

			if (AuthState != null)
				if (AuthState.User?.Identity?.IsAuthenticated ?? false)
					return true;
				else
					AuthState = null;

			if (AuthState == null) {
				SessionStorageUser SessionUser = await LoadSessionUser();

				if (Login(SessionUser, out AuthState)) {
					Console.WriteLine("Logged in via SessionUser");

					SetAuthenticationState(Task.FromResult(AuthState));
					return true;
				}
			}

			return false;
		}

		public bool Login(SessionStorageUser SessionUser, out AuthenticationState AuthState) {
			AuthState = null;

			if (SessionUser == null)
				return false;

			AuthState = CreateAuthState(SessionUser.Username, SessionUser.UserID);
			return true;
		}

		AuthenticationState CreateAuthState(string Username, int UserID) {
			List<Claim> Claims = new List<Claim>();
			Claims.Add(new Claim("Username", Username));
			Claims.Add(new Claim("UserID", UserID.ToString()));

			ClaimsIdentity Ident = new ClaimsIdentity(Claims, CookieAuthenticationDefaults.AuthenticationScheme);
			ClaimsPrincipal Princ = new ClaimsPrincipal(Ident);

			//SessionUser = new SessionStorageUser(DbUsr.Username, DbUsr.ID);
			return new AuthenticationState(Princ);
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
