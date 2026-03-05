Please refer to [kode-setup-guide.pdf](./kode-setup-guide.pdf) and [official website](https://github.com/shareAI-lab/Kode-Agent/tree/main) to learn how to configure Kode.

### the agent dir architecture

they are in `.clude` and `.kode` dir now.

```
.claude/
├── agents
│   ├── code-coverage-enhance.md
│   ├── fuzz-debug.md
│   ├── fuzz-debug-test.md
│   ├── opcode-test-enhance.md
│   ├── tests-fix.md
│   ├── tests-pipeline.md
│   ├── tests-review.md
│   └── tests-verify.md
├── commands
│   ├── batch-coverage-tasks.md
│   ├── batch-opcode-test.md
│   ├── enhance-coverage.md
│   ├── generate-opcode-test.md
│   ├── improve-coverage.md
│   └── opcode-enhance.md
└── skills
    ├── code-review.md
    └── cross-module-test.md
```

```
.kode
├── agents
│   ├── code-coverage-enhance.md
│   ├── coverage-plan-designer.md
│   ├── coverage-plan-reviewer.md
│   ├── feature-plan-designer.md
│   ├── feature-plan-reviewer.md
│   ├── opcode-test-enhance.md
│   └── plan-executor.md
└── commands
    ├── batch-cover-lines-tasks.md
    ├── batch-opcode-test.md
    ├── design_plan.md
    ├── fix-issue.md
    └── generate-opcode-test.md
```

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
