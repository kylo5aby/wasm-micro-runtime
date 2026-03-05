You are a WASM opcode test generation executor. Your role is to:

1. **Read the opcode queue** from the specified JSON configuration file
2. **Execute opcode test generation sequentially** by finding the next pending opcode
3. **Update opcode status** in the JSON file as you progress
4. **Clear context** between opcodes using `/clear`
5. **Provide progress updates** to the user

## Usage
`/batch-opcode-test <JSON_CONFIG_FILE>`

**Example:**
```bash
/batch-opcode-test opcode_batch.json
```
## JSON Configuration Format
```json
{
  "opcodes": [
    {
      "opcode": "i32.add",
      "status": "pending",
      "Start time": " ",
      "End time": " ",
      "commit_message": " ",
      "test_count": 0,
      "commit_hash": "",
      "notes": ""
    }
  ]
}
```

### Required Fields
- **opcode**: WASM opcode name (e.g., "i32.add", "br_if", "memory.grow")
- **status**: Processing status - `"pending"`, `"in_progress"`, `"completed"`, `"failed"`
- **execution_date**: Execution timestamp (YYYY-MM-DDTHH:MM:SS format)
- **commit_message**: Git commit message for this opcode
- **test_count**: Number of generated test cases (0 initially)
- **commit_hash**: Git commit hash (empty initially)
- **notes**: Execution notes and results (empty initially)

---

## Execution Workflow (MUST BE FOLLOWED EXACTLY):

### Phase 1: Initialize and Read Opcodes
1. Read JSON configuration file to get current opcode queue
2. Display current progress summary to user
3. Find the next pending opcode (status: "pending")
4. If no pending opcodes found, report completion

### Phase 2: Execute Next Opcode
1. **Update opcode status to "in_progress"** in JSON file
2. Display opcode information to user:
   ```
   🔄 EXECUTING OPCODE [opcode]
   📋 Opcode: [opcode_name]
   ⏰ Started: [current_timestamp]
   ```
3. **Execute the opcode test generation command**:
   `/opcode-enhance [opcode]`
4. **Capture execution results** from the command response
5. **Update JSON file** with results:
   - Status: "completed" or "failed"
   - Test count generated
   - Start time (format: YYYY-MM-DD HH:MM:SS)(Run date cmd to get current date)
   - End time (format: YYYY-MM-DD HH:MM:SS)(Run date cmd to get current date)

   - Commit hash (if available)
   - Notes from execution

### Phase 3: Progress and Continue
1. Display completion status:
   ```
   ✅ OPCODE [opcode] COMPLETED
   📊 Tests Generated: [test_count]
   📝 Status: [status]
   💾 Updated: [json_file]
   📁 Modified files: [files_modified]
   ```
2. **MUST** Run cmd clear to clear context window to free context window and reset our conversation history while keeping the current working directory and environment state, it must output command executed clearly for user visibility
3. **Continue to next pending opcode** (repeat from Phase 1) until all opcodes done

## Opcode Status Updates:

When updating the JSON file, ensure you:
- Set accurate status: "pending" → "in_progress" → "completed"/"failed"
- Update execution results: test_count, execution_date
- Add commit_hash if a commit was created
- Include meaningful notes about the execution

## Progress Reporting:

Always display to user:
- Current opcode being executed
- Overall progress (X of Y opcodes completed)
- Test generation statistics
- Any errors or issues encountered
- Next steps

## Error Handling:

If an opcode fails:
- Mark status as "failed"
- Document the failure reason in notes
- Continue to next opcode (don't stop the sequence)
- Report the failure to user clearly

**Execute opcodes one by one until all pending opcodes are completed, providing clear progress updates throughout the process.**

## Example JSON Configuration

### Complete Example
```json
{
  "opcodes": [
    {
      "opcode": "i32.add",
      "status": "pending",
      "Start time": "2024-11-02T14:30:45",
      "End time": "2024-11-02T14:40:55",
      "commit_message": "Enhanced unit tests for i32.add opcode - Comprehensive test coverage",
      "test_count": 0,
      "commit_hash": "",
      "notes": ""
    },
    {
      "opcode": "f64.div",
      "status": "completed",
      "Start time": "2024-11-02T14:30:45",
      "End time": "2024-11-02T14:40:55",
      "commit_message": "Enhanced unit tests for f64.div opcode - Comprehensive test coverage",
      "test_count": 12,
      "commit_hash": "f1a2b3c4d",
      "notes": "Successfully generated 12 test cases covering basic arithmetic, edge cases, and error conditions"
    }
  ]
}
```