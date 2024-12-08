target("ui")
    add_extrafiles("*.json")
    add_extrafiles("*.ts")
    add_extrafiles("*.js")
    add_extrafiles("*.html")
    add_extrafiles("src/**")
    add_extrafiles("public/**")
    on_build(function (target)
        local option = import("core.base.option")
        local oldir = os.cd(os.scriptdir())
        os.setenv("UI_BUILD_DIR", target:targetdir().."/dist")
        if os.host() == "windows" then
            if option.get("verbose") then
                os.exec("pnpm.cmd build --emptyOutDir")
            else
                os.run("pnpm.cmd build --emptyOutDir")
            end
        else
            if option.get("verbose") then
                os.exec("pnpm build --emptyOutDir")
            else
                os.run("pnpm build --emptyOutDir")
            end
        end
        os.cd(oldir)
    end)