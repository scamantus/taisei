
r_shaderlib_src = files(
    'cache.c',
    'lang_glsl.c',
    'lang_spirv_aux.c',
)

r_shaderlib_libdeps = []

if get_option('shader_transpiler')
    dep_spvc = dependency('spirv-cross-c-shared', required : true, static : static, fallback : ['SPIRV-Cross', 'spirv_cross_dep'])

    if get_option('wrap_mode') != 'forcefallback'
        dep_shaderc = dependency('shaderc', static : static, required : false)

        if not dep_shaderc.found()
            # Thanks, google.
            # https://github.com/google/shaderc/issues/392
            if static
                dep_shaderc = cc.find_library('shaderc_combined', required : false)
            else
                dep_shaderc = cc.find_library('shaderc_shared', required : false)
            endif
        endif
        if not dep_shaderc.found()
            dep_shaderc = dependency('shaderc', static : static, fallback : ['shaderc', 'shaderc_dep'], required : true)
        endif
    else
        dep_shaderc = dependency('shaderc', static : static, fallback : ['shaderc', 'shaderc_dep'], required : true)
    endif

    assert(dep_shaderc.found(), 'shaderc is required for the shader transpiler')

    r_shaderlib_src += files(
        'lang_spirv.c'
    )

    r_shaderlib_libdeps += [dep_shaderc, dep_spvc]

    if host_machine.system() == 'windows' and get_option('b_lto') and get_option('static')
        error(
    '''

    -!-  LTO is known to break glslang on Windows.
    -!-  Please disable it with `meson configure -Db_lto=false`.
    -!-  If you *really* want to use LTO, force it via compiler args or patch this check out.
    ''')
    endif
else
    r_shaderlib_src += files(
        'lang_spirv_stub.c'
    )
endif
