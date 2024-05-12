project('kiwi', 'cpp',
        default_options : 'default_library=static')

dependencies = [
    dependency('glfw3'),
    dependency('glew'),
    dependency('imgui'),
    dependency('glm'),
    dependency('spdlog'),
]

sources = [
    'main.cpp',
    'shader.cpp',
]

executable('kiwi', sources,
           dependencies : dependencies)