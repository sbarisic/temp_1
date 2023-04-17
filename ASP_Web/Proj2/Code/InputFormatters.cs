using Microsoft.AspNetCore.Mvc.Formatters;

namespace Proj2.Code {
	public class RawJsonBodyInputFormatter : InputFormatter {
		public RawJsonBodyInputFormatter() {
			SupportedMediaTypes.Add("application/json");
		}

		public override async Task<InputFormatterResult> ReadRequestBodyAsync(InputFormatterContext context) {
			HttpRequest Req = context.HttpContext.Request;

			using (StreamReader Reader = new StreamReader(Req.Body)) {
				string Content = await Reader.ReadToEndAsync();
				return await InputFormatterResult.SuccessAsync(Content);
			}
		}

		protected override bool CanReadType(Type T) {
			return T == typeof(string);
		}
	}
}
