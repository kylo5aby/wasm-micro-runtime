Please refer to [kode-setup-guide.pdf](./kode-setup-guide.pdf) and [official website](https://github.com/shareAI-lab/Kode-Agent/tree/main) to learn how to configure Kode.

### How to run

#### test generation

code coverage as example:

```
kode < .kode/code-coverage-enhance.md
```

#### test review

Prepare a `review_file_cc_list.md` first.

For example:

```
path/to/enhance_aot_runtime.cc
path/to_enhance_opecode_sub.cc
```

then run 

```
kode < .claude/test_pipeline.md
```

the agent will ask you for the input file which contains the unit test
files to be reviewd.

So you can input `review_file_cc_list.md`.
