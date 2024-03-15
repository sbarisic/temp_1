using Microsoft.AspNetCore.Cryptography.KeyDerivation;
using System.Security.Cryptography;

namespace Proj2.Code {
	public class PasswdManager : IDisposable {
		public byte[] GenerateSalt() {
			return RandomNumberGenerator.GetBytes(512 / 8);
		}

		public string GenerateHash(string Password, byte[] Salt) {
			if (Password == null) {
				throw new Exception("Password cannot be null");
			}

			if (Salt == null || Salt.Length == 0) {
				throw new Exception("Salt cannot be null or 0 length");
			}

			return Convert.ToBase64String(KeyDerivation.Pbkdf2(password: Password, salt: Salt, prf: KeyDerivationPrf.HMACSHA512, iterationCount: 100000, numBytesRequested: 512 / 8));
		}

		public void Dispose() {
		}
	}
}
