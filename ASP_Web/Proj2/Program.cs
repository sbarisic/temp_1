using Microsoft.AspNetCore.Components.Authorization;
using Microsoft.AspNetCore.ResponseCompression;
using Microsoft.EntityFrameworkCore;
using Microsoft.AspNetCore.HttpOverrides;
using Proj2.Code;
using Proj2.Database;
using Proj2.Hubs;

var builder = WebApplication.CreateBuilder(args);
builder.Configuration.AddJsonFile("appsettings.json", false);
builder.Configuration.AddJsonFile("appsettings.prod.json", true);

// Add services to the container.
builder.Services.AddRazorPages();
builder.Services.AddServerSideBlazor();

builder.Services.AddSingleton<ConfigurationService>();

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

builder.Services.AddControllers();
var app = builder.Build();
app.Services.GetService<ConfigurationService>();
app.UseResponseCompression();



// Configure the HTTP request pipeline.
if (!app.Environment.IsDevelopment()) {
	Console.WriteLine("Using Production Environment");
	app.UseExceptionHandler("/Error");
	// The default HSTS value is 30 days. You may want to change this for production scenarios, see https://aka.ms/aspnetcore-hsts.
	app.UseHsts();
} else {
	Console.WriteLine("Using Development Environment");
}

app.UseForwardedHeaders(new ForwardedHeadersOptions {
	ForwardedHeaders = ForwardedHeaders.XForwardedFor | ForwardedHeaders.XForwardedProto
});

// app.UseHttpsRedirection();
app.UseStaticFiles();
app.UseRouting();


app.MapBlazorHub();
app.MapHub<ComHub>("/comhub");
app.MapFallbackToPage("/_Host");
app.MapControllers();

//app.MapControllerRoute(name: "default", pattern: "{controller=Home}/{action=Index}/{id?})");


app.Run();
