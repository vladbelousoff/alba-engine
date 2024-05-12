project('kiwi', 'cpp',
        default_options : 'default_library=static')

dependencies = [
    dependency('glfw3'),
    dependency('glew'),
    dependency('imgui'),
    dependency('glm'),
    dependency('spdlog'),
    dependency('gtest'),
]

sources = [
    'main.cpp',
    'shader.cpp',
    'string_manager.cpp',
]

executable('kiwi', sources,
           dependencies : dependencies)