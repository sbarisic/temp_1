function core2_generate(template_name, obj) {
    let temp = document.querySelector(template_name).innerHTML;
    let temp_func = Handlebars.compile(temp);

    return temp_func(obj);
}

function core2_post(location, cmd) {
    let url = window.location.origin;

    return fetch(url + location, {
        method: "POST",
        body: cmd
    });
}

//=============================================================================

var shell_variables = [];
var generated_content = [];

function core2_js_main() {
    console.log("Core2 JS Main();");

    shell_var = [];
    generated_content = [];

    core2_post("/shell", "get_variables").then((val) => val.json()).then((val) => {
        val.vars.forEach(element => {
            let shell_var = {
                variable_name: element.variable_name,
                input_type: "text",
                value: element.value ??= ""
            };

            shell_variables.push(shell_var);
            generated_content.push(core2_generate("#input_temp", shell_var));

            document.querySelector("#generated_content").innerHTML = generated_content.join("");
        });
    });
}

function core2_onSave() {
    shell_variables.forEach(element => {
        console.log(element);
    });

    alert("Save!");
}