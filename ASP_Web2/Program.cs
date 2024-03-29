using Microsoft.AspNetCore.Components.Authorization;
using Microsoft.AspNetCore.ResponseCompression;
using Microsoft.EntityFrameworkCore;
using Microsoft.AspNetCore.HttpOverrides;
using Proj2.Code;
using Proj2.Database;
using Proj2.Hubs;
using Blazored.Modal;
using System.Text.Json.Serialization;

namespace Proj2
{
	public class Program
	{
		public static void Main(string[] args)
		{
            WebApplicationBuilder builder = WebApplication.CreateBuilder(args);
            builder.Configuration.AddJsonFile("appsettings.json", false);
            builder.Configuration.AddJsonFile("appsettings.prod.json", true);

            // Add services to the container.
            builder.Services.AddRazorPages();
            builder.Services.AddServerSideBlazor();

            builder.Services.AddSingleton<ConfigurationService>();
            builder.Services.AddSingleton<ComHubService>();

            builder.Services.AddDbContext<DatabaseContext>();


            builder.Services.AddResponseCompression(opts => {
                opts.MimeTypes = ResponseCompressionDefaults.MimeTypes.Concat(
                    new[] { "application/octet-stream" });
            });

            builder.Services.AddScoped<AuthStateProvider>();
            //builder.Services.AddScoped<DatabaseService>();

            builder.Services.AddScoped<IHostEnvironmentAuthenticationStateProvider>(sp => {
                AuthStateProvider AuthProvider = sp.GetRequiredService<AuthStateProvider>();
                return AuthProvider;
            });

            builder.Services.AddScoped<AuthenticationStateProvider>(sp => {
                AuthStateProvider AuthProvider = sp.GetRequiredService<AuthStateProvider>();
                return AuthProvider;
            });

            builder.Services.AddHttpContextAccessor();
            builder.Services.AddControllers((opts) => {
                opts.InputFormatters.Insert(0, new RawJsonBodyInputFormatter());
            }).AddJsonOptions(opts => {
                opts.JsonSerializerOptions.DefaultIgnoreCondition = JsonIgnoreCondition.WhenWritingNull;
            });

            builder.Services.AddBlazoredModal();
            builder.Services.AddAuthentication("Cookies").AddCookie();

            WebApplication app = builder.Build();
            app.Services.GetService<ConfigurationService>();
            app.Services.GetService<ComHubService>();
            app.UseResponseCompression();



            // Configure the HTTP request pipeline.
            if (!app.Environment.IsDevelopment())
            {
                Console.WriteLine("Using Production Environment");
                app.UseExceptionHandler("/Error");
                // The default HSTS value is 30 days. You may want to change this for production scenarios, see https://aka.ms/aspnetcore-hsts.
                app.UseHsts();
            }
            else
            {
                Console.WriteLine("Using Development Environment");
            }

            app.UseForwardedHeaders(new ForwardedHeadersOptions
            {
                ForwardedHeaders = ForwardedHeaders.XForwardedFor | ForwardedHeaders.XForwardedProto
            });

            // app.UseHttpsRedirection();
            app.UseStaticFiles();
            app.UseRouting();

            app.UseAuthentication();
            app.UseAuthorization();

            app.MapBlazorHub();
            app.MapHub<ComHub>("/comhub");
            app.MapFallbackToPage("/_Host");
            //app.MapControllers();

            app.UseEndpoints(Endpoints => {
                Endpoints.MapControllers();
            });

            //app.MapControllerRoute(name: "default", pattern: "{controller=Home}/{action=Index}/{id?})");

#if DEBUG
            using (DatabaseContext DbCtx = new DatabaseContext())
            {
                string CreateScript = DbCtx.GenerateCreateScript();
            }
#endif


            app.Run();
        }
	}
}
