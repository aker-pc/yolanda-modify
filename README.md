# yolanda-modify
- 注意事项
  - 本项目仅添加了部分注释，便于理解整体逻辑，有相关的疑问小伙伴欢迎一起讨论呀
  - 仅用于学习理解，课程很棒，切记不要付费分享呀。欢迎购买老师的课啦
  - 原仓库地址：https://github.com/froghui/yolanda
  - 课程地址：https://b.geekbang.org/member/course/detail/129807
- 此外，原有课程代码会出现编译不通过的情况（该版本已修改）
  - 原始版本：`target_link_libraries(aio01 yolanda)`
  - 改动为：`target_link_libraries(aio01 yolanda rt)`
