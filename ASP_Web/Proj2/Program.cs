using BlazorStrap;
using Microsoft.AspNetCore.Components;
using Microsoft.AspNetCore.Components.Authorization;
using Microsoft.AspNetCore.Components.Server;
using Microsoft.AspNetCore.Components.Web;
using Microsoft.AspNetCore.Identity;
using Microsoft.AspNetCore.ResponseCompression;
using Microsoft.EntityFrameworkCore;
using Proj2.Code;
using Proj2.Data;
using Proj2.Database;
using Proj2.Hubs;

var builder = WebApplication.CreateBuilder(args);

// Add services to the container.
builder.Services.AddRazorPages();
builder.Services.AddServerSideBlazor();
builder.Services.AddSingleton<ItemDataService>();
builder.Services.AddSingleton<DatabaseService>();

builder.Services.AddResponseCompression(opts => {
    opts.MimeTypes = ResponseCompressionDefaults.MimeTypes.Concat(
        new[] { "application/octet-stream" });
});
builder.Services.AddBlazorStrap();

builder.Services.AddScoped<AuthenticationStateProvider, AuthStateProvider>();

builder.Services.AddScoped<IHostEnvironmentAuthenticationStateProvider>(sp => {
	var provider = (AuthStateProvider)sp.GetRequiredService<AuthenticationStateProvider>();
	return provider;
});

builder.Services.AddScoped<AuthStateProvider>(sp => {
	var provider = (AuthStateProvider)sp.GetRequiredService<AuthenticationStateProvider>();
	return provider;
});

var app = builder.Build();
app.Services.GetService<DatabaseService>();

app.UseResponseCompression();


// Configure the HTTP request pipeline.
if (!app.Environment.IsDevelopment()) {
    app.UseExceptionHandler("/Error");
    // The default HSTS value is 30 days. You may want to change this for production scenarios, see https://aka.ms/aspnetcore-hsts.
    app.UseHsts();
}

app.UseHttpsRedirection();
app.UseStaticFiles();
app.UseRouting();

app.MapBlazorHub();
app.MapHub<ComHub>("/comhub");
app.MapFallbackToPage("/_Host");


app.Run();
