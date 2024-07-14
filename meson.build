project('loki', 'cpp',
        default_options : [
            'default_library=static',
            'cpp_std=c++17',
            'cpp_rtti=false',
        ])

dependencies = [
    dependency('glfw3'),
    dependency('glew'),
    dependency('imgui'),
    dependency('glm'),
    dependency('spdlog'),
    dependency('gtest'),
    dependency('cli11'),
    dependency('mimalloc'),
    dependency('eastl'),
    dependency('stormlib'),
]

sources = [
    'engine/main.cpp',
    'engine/engine_application.cpp',
    'engine/string_manager.cpp',
    'engine/render/shader.cpp',
    'project/project_application.cpp',
]

executable('loki', sources,
           dependencies : dependencies)