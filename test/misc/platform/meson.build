# wlr_platform_test - disabled, depends on wlroots wlr_platform API which doesn't exist in wlroots 0.20.x
#wlr_platform_test = executable(
#    'wlr_platform_test',
#    'platform-test.cpp',
#    dependencies: [doctest, wlroots],
#    install: false)
#test('wlr_platform abstraction test', wlr_platform_test)

wf_platform_test = executable(
    'wf_platform_test',
    'wayfire-platform-test.cpp',
    dependencies: [doctest, libwayfire],
    install: false)
test('wayfire platform backend test', wf_platform_test)
