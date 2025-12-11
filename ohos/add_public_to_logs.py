#!/usr/bin/env python3
"""
自动为 OHOS_LOG 宏中的格式字符串添加 {public} 标记
"""

import re
import sys
from pathlib import Path

# 格式说明符映射
FORMAT_SPECS = [
    (r'%([+-]?\d*\.?\d*)d', r'%\1{public}d'),      # %d, %5d, %.2d
    (r'%([+-]?\d*\.?\d*)i', r'%\1{public}i'),      # %i
    (r'%([+-]?\d*\.?\d*)u', r'%\1{public}u'),      # %u
    (r'%([+-]?\d*\.?\d*)x', r'%\1{public}x'),      # %x
    (r'%([+-]?\d*\.?\d*)X', r'%\1{public}X'),      # %X
    (r'%([+-]?\d*\.?\d*)o', r'%\1{public}o'),      # %o
    (r'%([+-]?\d*\.?\d*)f', r'%\1{public}f'),      # %f, %.2f
    (r'%([+-]?\d*\.?\d*)F', r'%\1{public}F'),      # %F
    (r'%([+-]?\d*\.?\d*)e', r'%\1{public}e'),      # %e
    (r'%([+-]?\d*\.?\d*)E', r'%\1{public}E'),      # %E
    (r'%([+-]?\d*\.?\d*)g', r'%\1{public}g'),      # %g
    (r'%([+-]?\d*\.?\d*)G', r'%\1{public}G'),      # %G
    (r'%([+-]?\d*\.?\d*)ld', r'%\1{public}ld'),    # %ld
    (r'%([+-]?\d*\.?\d*)lu', r'%\1{public}lu'),    # %lu
    (r'%([+-]?\d*\.?\d*)lld', r'%\1{public}lld'),  # %lld
    (r'%([+-]?\d*\.?\d*)llu', r'%\1{public}llu'),  # %llu
    (r'%([+-]?\d*\.?\d*)zu', r'%\1{public}zu'),    # %zu (size_t)
    (r'%([+-]?\d*\.?\d*)s', r'%\1{public}s'),      # %s
    (r'%p', r'%{public}p'),                         # %p (pointer)
]

def add_public_to_format_string(format_str):
    """
    为格式字符串中的所有格式说明符添加 {public} 标记
    跳过已经有 {public} 的
    """
    # 如果已经包含 {public}，跳过
    if '{public}' in format_str:
        return format_str
    
    result = format_str
    for pattern, replacement in FORMAT_SPECS:
        # 确保不会重复添加 {public}
        result = re.sub(pattern, replacement, result)
    
    return result

def process_ohos_log_call(match):
    """
    处理 OHOS_LOG 宏调用
    """
    log_macro = match.group(1)  # OHOS_LOGI, OHOS_LOGE, etc.
    tag = match.group(2)         # TAG
    format_str = match.group(3)  # 格式字符串
    rest = match.group(4)         # 剩余参数
    
    # 处理格式字符串
    new_format_str = add_public_to_format_string(format_str)
    
    # 重新组装
    return f'{log_macro}({tag}, {new_format_str}{rest})'

def process_file(file_path, dry_run=False):
    """
    处理单个文件
    """
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
    except Exception as e:
        print(f"Error reading {file_path}: {e}")
        return False
    
    # 匹配 OHOS_LOG 宏调用
    # OHOS_LOGI(TAG, "format %d", arg)
    pattern = r'(OHOS_LOG[DIWE])\(([^,]+),\s*("(?:[^"\\]|\\.)*")([^)]*)\)'
    
    new_content = re.sub(pattern, process_ohos_log_call, content)
    
    if new_content != content:
        if dry_run:
            print(f"Would modify: {file_path}")
            # 显示差异
            lines_old = content.split('\n')
            lines_new = new_content.split('\n')
            for i, (old, new) in enumerate(zip(lines_old, lines_new), 1):
                if old != new:
                    print(f"  Line {i}:")
                    print(f"    - {old}")
                    print(f"    + {new}")
        else:
            try:
                with open(file_path, 'w', encoding='utf-8') as f:
                    f.write(new_content)
                print(f"Modified: {file_path}")
            except Exception as e:
                print(f"Error writing {file_path}: {e}")
                return False
        return True
    else:
        print(f"No changes needed: {file_path}")
        return False

def main():
    import argparse
    
    parser = argparse.ArgumentParser(description='Add {public} to OHOS_LOG format strings')
    parser.add_argument('files', nargs='*', help='Files to process (default: all .cpp files in ohos/)')
    parser.add_argument('--dry-run', action='store_true', help='Show what would be changed without modifying files')
    
    args = parser.parse_args()
    
    if args.files:
        files = [Path(f) for f in args.files]
    else:
        # 默认处理 ohos/ 目录下的所有 .cpp 文件
        ohos_dir = Path(__file__).parent / 'entry' / 'src' / 'main' / 'cpp'
        if not ohos_dir.exists():
            ohos_dir = Path('ohos/entry/src/main/cpp')
        
        files = list(ohos_dir.rglob('*.cpp'))
        files.extend(ohos_dir.rglob('*.h'))
    
    print(f"Processing {len(files)} files...")
    if args.dry_run:
        print("DRY RUN MODE - no files will be modified")
    print()
    
    modified_count = 0
    for file_path in files:
        if process_file(file_path, args.dry_run):
            modified_count += 1
    
    print()
    print(f"Summary: {modified_count} files {'would be' if args.dry_run else 'were'} modified")

if __name__ == '__main__':
    main()
