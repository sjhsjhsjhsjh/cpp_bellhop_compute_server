%% ========== Bellhop C++ vs MATLAB Verification Script ==========
% 这个脚本用于验证C++多线程版本和MATLAB版本的Bellhop计算结果是否一致
%
% 功能：
% 1. 批量执行MATLAB版本的Bellhop计算（对应build/Release中的env_*.env文件）
% 2. 提取TL数据并按C++格式输出
% 3. 自动对比C++和MATLAB的计算结果
% 4. 生成详细的对比报告
%
% 使用说明：
%   1. 确保MATLAB Bellhop工具箱在搜索路径中
%   2. 确保build/Release目录存在且包含env_*.env和corresponding .bty文件
%   3. 运行此脚本：runtest_batch
%
% 输出：
%   - matlab_output/tl_*.txt          - MATLAB版本的TL输出文件
%   - matlab_output/comparison_report.txt - 对比报告

clear; clc;
fprintf('==================== Bellhop Verification Suite ====================\n\n');

%% 配置
env_dir = '.';  % env文件所在目录
output_dir = 'matlab_output';  % MATLAB输出目录
report_file = fullfile(output_dir, 'comparison_report.txt');  % 报告文件

%% Step 1: 创建输出目录
if ~exist(output_dir, 'dir')
    mkdir(output_dir);
    fprintf('[INFO] Created output directory: %s\n', output_dir);
end

%% Step 2: 批量处理env文件
fprintf('\n========== PHASE 1: Batch Processing ==========\n');
fprintf('Processing all env files in: %s\n\n', env_dir);

try
    batch_process_env_files(env_dir, output_dir);
    fprintf('\n[SUCCESS] Batch processing completed\n');
catch ME
    fprintf('\n[ERROR] Batch processing failed: %s\n', ME.message);
    rethrow(ME);
end

%% Step 3: 对比验证
fprintf('\n========== PHASE 2: Comparison & Verification ==========\n');
fprintf('Comparing C++ and MATLAB results...\n\n');

try
    compare_results(env_dir, output_dir, report_file);
    fprintf('\n[SUCCESS] Comparison completed\n');
catch ME
    fprintf('\n[ERROR] Comparison failed: %s\n', ME.message);
    rethrow(ME);
end

%% Step 4: 显示报告
fprintf('\n========== PHASE 3: Report Summary ==========\n');
fprintf('Full report saved to: %s\n', report_file);
fprintf('\nShowing report content:\n');
fprintf('---------------------------------------------\n');

try
    report_text = fileread(report_file);
    fprintf('%s\n', report_text);
catch ME
    fprintf('[WARNING] Could not read report file: %s\n', ME.message);
end

fprintf('\n==================== Verification Complete ====================\n');
fprintf('Next steps:\n');
fprintf('  1. Review the comparison_report.txt for detailed results\n');
fprintf('  2. Check matlab_output/ for all generated tl_*.txt files\n');
fprintf('  3. If differences exist, examine specific files for debugging\n\n');
