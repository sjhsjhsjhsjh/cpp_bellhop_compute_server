%% ========== Single File Test Script ==========
% 这个脚本用于测试单个env文件的计算流程
% 推荐先运行这个脚本验证整个流程是否正常工作
%
% 使用说明：
%   1. 修改 test_fileroot 变量为你要测试的env文件的基名
%   2. 运行此脚本
%   3. 检查 matlab_output/tl_<fileroot>.txt 是否生成正确

clear; clc;
fprintf('==================== Single File Test ====================\n\n');

%% 配置
test_fileroot = 'env_1d15869ed451f6d9bd8a45e2c745e5b2';  % 修改这里
env_dir = '.';  % env文件所在目录（当前目录）
output_dir = 'matlab_output';  % 输出目录

fprintf('Testing file: %s\n', test_fileroot);
fprintf('Env directory: %s\n', env_dir);
fprintf('Output directory: %s\n\n', output_dir);

%% 创建输出目录
if ~exist(output_dir, 'dir')
    mkdir(output_dir);
end

%% Step 1: 解析env文件
fprintf('[STEP 1] Parsing env file...\n');
env_file = fullfile(env_dir, [test_fileroot '.env']);

try
    config = parse_env(env_file);
    fprintf('[SUCCESS] Env file parsed successfully\n\n');
catch ME
    fprintf('[ERROR] Failed to parse env file: %s\n', ME.message);
    return;
end

%% Step 2: 执行bellhop3d计算
fprintf('[STEP 2] Running bellhop3d calculation...\n');
fprintf('  (This may take a moment...)\n');

try
    bellhop3d(test_fileroot);
    fprintf('[SUCCESS] bellhop3d calculation completed\n\n');
catch ME
    fprintf('[ERROR] bellhop3d calculation failed: %s\n', ME.message);
    return;
end

%% Step 3: 提取TL数据
fprintf('[STEP 3] Extracting TL data and writing output...\n');

try
    extract_and_write_tl(test_fileroot, config, output_dir);
    fprintf('[SUCCESS] TL data extracted and written\n\n');
catch ME
    fprintf('[ERROR] TL extraction failed: %s\n', ME.message);
    return;
end

%% Step 4: 验证输出文件
fprintf('[STEP 4] Verifying output file...\n');
output_file = fullfile(output_dir, sprintf('tl_%s.txt', test_fileroot));

if isfile(output_file)
    % 读取输出文件，显示一些基本统计
    data = readmatrix(output_file);
    fprintf('[INFO] Output file created successfully\n');
    fprintf('  File: %s\n', output_file);
    fprintf('  Total data rows: %d (excluding header)\n', size(data, 1) - 1);
    fprintf('  Header (first row): ');
    fprintf('%.4f ', data(1, 1:min(6, size(data,2))));
    fprintf('\n');
    
    % 显示前5行数据
    fprintf('  First 5 data rows:\n');
    for i = 2 : min(6, size(data, 1))
        fprintf('    %.4f %.4f %.4f %.4f\n', data(i, 1), data(i, 2), data(i, 3), data(i, 4));
    end
else
    fprintf('[ERROR] Output file not found: %s\n', output_file);
    return;
end

%% Step 5: 比较C++版本（如果存在）
fprintf('\n[STEP 5] Comparing with C++ version...\n');
% C++输出文件应该在当前目录（build/Release）中
cpp_filename = sprintf('tl_%s.txt', test_fileroot(5:end));
cpp_file = cpp_filename;
fprintf('Finding C++ output file: %s\n', cpp_file);

if isfile(cpp_file)
    fprintf('  C++ file found: %s\n', cpp_file);
    
    try
        result = compare_single_file(cpp_file, output_file);
        
        fprintf('\n  Comparison result:\n');
        fprintf('    C++ output rows: %d\n', result.cpp_lines);
        fprintf('    MATLAB output rows: %d\n', result.matlab_lines);
        
        if result.is_identical
            fprintf('    ✓ IDENTICAL - Results match perfectly!\n');
        else
            fprintf('    Matching rows: %d/%d (%.2f%%)\n', ...
                result.matching_rows, result.total_rows, ...
                result.matching_rows / max(result.total_rows, 1) * 100);
            fprintf('    Max TL difference (dB): %.6f\n', result.max_tl_diff);
            fprintf('    Mean TL difference (dB): %.6f\n', result.mean_tl_diff);
        end
        
    catch ME
        fprintf('  [WARNING] Comparison failed: %s\n', ME.message);
    end
else
    fprintf('  [INFO] C++ file not found. Run C++ version first for comparison.\n');
end

fprintf('\n==================== Test Complete ====================\n');
fprintf('\nNext steps:\n');
fprintf('  1. Review the output file: %s\n', output_file);
fprintf('  2. If test passed, run runtest_batch.m for batch processing\n');
fprintf('  3. For debugging, check the .shd file with MATLAB Bellhop visualization\n\n');
