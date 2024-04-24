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

function core2_js_main() {
    console.log("Core2 JS Main();");

    core2_post("/shell", "get_variables").then((val) => {
        console.log("Value:");
        console.log(val);
    });



    let generated_content = [];

    core2_post("/shell", "get_variables").then((val) => val.json()).then((val) => {
        val.vars.forEach(element => {
            generated_content.push(core2_generate("#input_temp", {
                variable_name: element.variable_name,
                input_type: "text",
                value: element.value ??= ""
            }));

            document.querySelector("#generated_content").innerHTML = generated_content.join("");
        });
    });

    /*generated_content.push(core2_generate("#input_temp", {
        variable_name: "Variable 1",
        input_type: "text",
        value: "Variable Value"
    }));

    generated_content.push(core2_generate("#input_temp", {
        variable_name: "Variable 2",
        input_type: "password",
        value: "123456"
    }));*/

    
}

function core2_onSave() {
    alert("Save!");
}