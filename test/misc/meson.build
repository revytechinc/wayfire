tracking_allocator = executable(
    'tracking_allocator',
    'tracking-allocator.cpp',
    dependencies: libwayfire,
    install: false)
test('Tracking factory test', tracking_allocator)

safe_list = executable(
    'safe_list',
    'safe-list-test.cpp',
    dependencies: [doctest, wfconfig],
    install: false)
test('Safe list test', safe_list)

object_signal = executable(
    'object-signal-test',
    'object-signal-test.cpp',
    dependencies: libwayfire,
    install: false)
test('Object and signal test', object_signal)

nonstd_test = executable(
    'nonstd_test',
    'nonstd-test.cpp',
    dependencies: [doctest, libwayfire],
    install: false)
test('Nonstd utilities test', nonstd_test)

variant_test = executable(
    'variant_test',
    'variant-test.cpp',
    dependencies: [doctest, libwayfire],
    install: false)
test('Variant utilities test', variant_test)

lexer_test = executable(
    'lexer_test',
    'lexer-test.cpp',
    dependencies: [doctest, libwayfire],
    install: false)
test('Lexer utilities test', lexer_test)

condition_test = executable(
    'condition_test',
    'condition-test.cpp',
    dependencies: [doctest, libwayfire],
    install: false)
test('Condition utilities test', condition_test)
