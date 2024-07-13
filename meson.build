project('alba', 'cpp',
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
]

sources = [
    'main.cpp',
    'engine/engine_application.cpp',
    'project/project_application.cpp',
    'shader.cpp',
    'string_manager.cpp',
    'mt/spin_lock.cpp',
    'mt/job.cpp',
    'mt/job_queue.cpp',
    'mt/worker_thread.cpp',
    'mt/thread_pool.cpp',
    'ecs/world.cpp',
    'ecs/component.cpp',
    'ecs/archetype.cpp',
]

executable('alba', sources,
           dependencies : dependencies)