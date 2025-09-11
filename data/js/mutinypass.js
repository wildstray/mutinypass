var zonesFile = "/config/zones.json";
var configFile = "/config/mutinypass.json";

$( document ).ready(function() {
    // configuration form part for time zone selection
    $.get(zonesFile)
        .done(function(data, textStatus, jqXHR) {
            $.each(data, function(key, value) {
                $("#tzname").append(`<option value="${key}">${key}</option>`);
            });
            $('#tzname').on('change', '', function (e) {
                $('#tzconfig').val(data[this.value]);
            });
        })
        .fail(function(jqXHR, textStatus, errorThrown) {
            console.log(jqXHR);
            console.log(textStatus);
            console.log(errorThrown);
        });
    $("body").on("click", ".remove", function () {
        $(this).parent().remove();
    });
    $(".form-account").on("click", ".add", function () {
        $(".form-account").last().before($(".empty-account").html().replace("btn-success", "btn-danger").replace("add", "remove").replace("+","-"));
    });
    $(".form-otp").on("click", ".add", function () {
        $(".form-otp").last().before($(".empty-otp").html().replace("btn-success", "btn-danger").replace("add", "remove").replace("+","-"));
    });
    $.get({url: configFile, cache: false})
        .done(function(data, textStatus, jqXHR) {
            // configuration form for accounts and action buttons
            $.each(data.accounts, function(key, value) {
                var button = `<div class="col sm d-flex justify-content-center"><input class="btn btn-primary" type="button" name="${value.uuid}" id="${value.uuid}" value="${value.name}" /></div>`;
                $(".form-actions").append(button);
                $(`#${value.uuid}`).on("click", function () {
                    $.get({url: `send?uuid=${value.uuid}`, cache: false})
                        .done(function(data, textStatus, jqXHR) {
                            console.log(data);
                        })
                        .fail(function(jqXHR, textStatus, errorThrown) {
                            console.log(jqXHR);
                            console.log(textStatus);
                            console.log(errorThrown);
                        });
                });
            });
            $.each(data.accounts, function(key, value) {
                $(".form-account").last().before($(".empty-account").prop('outerHTML').replace("btn-success", "btn-danger").replace("add", "remove").replace("+","-").replace("empty-account",value.uuid));
            });
            $.each(data.otp, function(key, value) {
                $(".form-account").find("#otp").append(`<option value="${value.uuid}">${value.name}</option>`);
            });
            $.each(data.accounts, function(key, value) {
                $(`.${value.uuid}`).autofill(value);
            });
            // configuration form for otp
            $.each(data.otp, function(key, value) {
                $(".form-otp").last().before($(".empty-otp").prop('outerHTML').replace("btn-success", "btn-danger").replace("add", "remove").replace("+","-").replace("empty-otp",value.uuid));
                $(`.${value.uuid}`).autofill(value);
            });
            // configuration form for all other parameters
            $("#form-config").autofill(data);
    })
    .fail(function(jqXHR, textStatus, errorThrown) {
        console.log(jqXHR);
        console.log(textStatus);
        console.log(errorThrown);
    });
});
