using Microsoft.AspNetCore.Mvc;
using Microsoft.AspNetCore.Mvc.RazorPages;
using Microsoft.AspNetCore.Mvc.Rendering;

namespace Proj2.Models {
	public class PregledUpraveModel : PageModel {

		public PregledUpraveModel() {

		}

		[BindProperty(SupportsGet = true)]
		public string Uprava {
			get; set;
		}
	}
}
