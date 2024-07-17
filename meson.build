project('loki', 'cpp',
        default_options : [
            'default_library=static',
            'cpp_std=c++17',
        ])

# Define the Winsock library dependency
winsock_dep = declare_dependency(link_args : ['-lWs2_32'])

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
    dependency('sockpp'),
    winsock_dep,
]

sources = [
    'engine/main.cpp',
    'engine/engine_application.cpp',
    'engine/string_manager.cpp',
    'engine/render/shader.cpp',
    'engine/datasource/mpq/mpq_archive.cpp',
    'engine/datasource/mpq/mpq_chain.cpp',
    'engine/datasource/mpq/mpq_file.cpp',
    'project/project_application.cpp',
]

executable('loki', sources,
           dependencies : dependencies)