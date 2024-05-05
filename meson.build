project('kiwi', 'cpp',
        default_options: 'default_library=static')

dependencies = [
    dependency('glfw3'),
    dependency('glew'),
    dependency('imgui')
]

executable('kiwi', 'main.cpp',
           dependencies: dependencies)