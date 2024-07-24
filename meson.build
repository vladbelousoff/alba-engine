project('loki', 'cpp',
        default_options : [
            'default_library=static',
            'cpp_std=c++17',
        ])

# Define the Winsock library dependency

dependencies = [
    dependency('glfw3'),
    dependency('glew'),
    dependency('imgui'),
    dependency('glm'),
    dependency('spdlog'),
    dependency('cli11'),
    dependency('mimalloc'),
    dependency('stormlib'),
    dependency('sockpp'),
    dependency('glob'),
    dependency('libassert'),
    dependency('cpptrace'),
]

c_compiler = meson.get_compiler('c')

if host_machine.system() == 'windows'
    dependencies += [declare_dependency(link_args : ['-lWs2_32'])]
    dependencies += [c_compiler.find_library('dbghelp')]
endif

sources = [
    'engine/main.cpp',
    'engine/engine_application.cpp',
    'engine/string_manager.cpp',
    'engine/network/byte_buffer.cpp',
    'engine/network/packet.cpp',
    'engine/render/shader.cpp',
    'engine/datasource/mpq/mpq_archive.cpp',
    'engine/datasource/mpq/mpq_chain.cpp',
    'engine/datasource/mpq/mpq_file.cpp',
    'project/project_application.cpp',
]

executable('loki', sources,
           dependencies : dependencies)