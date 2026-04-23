function compare_results(cpp_dir, matlab_dir, report_file)
% 对比C++和MATLAB版本的TL计算结果
%
% Usage: compare_results(cpp_dir, matlab_dir, report_file)
%
% Input:
%   cpp_dir    - C++输出目录（包含tl_*.txt文件），默认：build/Release
%   matlab_dir - MATLAB输出目录，默认：matlab_output
%   report_file - 对比报告输出文件，默认：matlab_output/comparison_report.txt
%
% Example:
%   compare_results('build/Release', 'matlab_output', 'matlab_output/comparison_report.txt')
%
% Notes:
%   - 生成详细的逐文件对比报告
%   - 统计总体一致性指标

if nargin < 1
    cpp_dir = '.';
end

if nargin < 2
    matlab_dir = 'matlab_output';
end

if nargin < 3
    report_file = fullfile(matlab_dir, 'comparison_report.txt');
end

% 扫描C++输出目录
fprintf('[INFO] compare_results: Scanning C++ output directory: %s\n', cpp_dir);
% cpp_files = dir(fullfile(cpp_dir, 'tl_*.txt'));
cpp_files = dir(fullfile('tl_*.txt'));

if isempty(cpp_files)
    error('compare_results: No tl_*.txt files found in %s', cpp_dir);
end

fprintf('[INFO] compare_results: Found %d files to compare\n', length(cpp_files));

% 打开报告文件
report_fid = fopen(report_file, 'w');
if report_fid == -1
    error('compare_results: Cannot open report file: %s', report_file);
end

% 写入报告头
fprintf(report_fid, '========== TL COMPUTATION COMPARISON REPORT ==========\n');
fprintf(report_fid, 'C++ Output Dir: %s\n', cpp_dir);
fprintf(report_fid, 'MATLAB Output Dir: %s\n', matlab_dir);
fprintf(report_fid, 'Generated at: %s\n', datetime('now'));
fprintf(report_fid, '=====================================================\n\n');

% 统计指标
total_files = length(cpp_files);
identical_files = 0;
diff_files = 0;
missing_files = 0;

% 详细对比信息
all_diffs = [];

% 处理每个C++输出文件
for i = 1 : total_files
    cpp_filename = cpp_files(i).name;
    cpp_filepath = fullfile(cpp_dir, cpp_filename);
    % 对应的MATLAB文件
    % 将tl_替换为tl_env_
    matlab_file_name = char(cpp_files(i).name);
    matlab_file_name = matlab_file_name(4:end);
    matlab_file_name = ['tl_env_' , matlab_file_name];
    matlab_filepath = fullfile(matlab_dir, matlab_file_name);

    % 检查MATLAB文件是否存在
    if ~isfile(matlab_filepath)
        fprintf('[WARNING] compare_results: Missing MATLAB output for %s\n', cpp_filename);
        fprintf(report_fid, '[MISSING] %s\n', cpp_filename);
        missing_files = missing_files + 1;
        continue;
    end
    
    % 对比两个文件
    try
        result = compare_single_file(cpp_filepath, matlab_filepath);
        
        % 写入文件级别的对比结果
        if result.is_identical
            identical_files = identical_files + 1;
            fprintf(report_fid, '[✓ IDENTICAL] %s\n', cpp_filename);
        else
            diff_files = diff_files + 1;
            fprintf(report_fid, '[✗ DIFF] %s\n', cpp_filename);
            fprintf(report_fid, '  Lines: %d (C++) vs %d (MATLAB)\n', result.cpp_lines, result.matlab_lines);
            fprintf(report_fid, '  Matching rows: %d/%d (%.2f%%)\n', ...
                result.matching_rows, result.total_rows, ...
                result.matching_rows / max(result.total_rows, 1) * 100);
            
            if result.total_rows > 0
                fprintf(report_fid, '  Max TL diff (dB): %.6f\n', result.max_tl_diff);
                fprintf(report_fid, '  Mean TL diff (dB): %.6f\n', result.mean_tl_diff);
                fprintf(report_fid, '  Max range diff (%%): %.6f\n', result.max_range_pct_diff);
                fprintf(report_fid, '  Max bearing diff (%%): %.6f\n', result.max_bearing_pct_diff);
            end
            fprintf(report_fid, '\n');
            
            % 记录详细信息供后续分析
            all_diffs = [all_diffs; result];
        end
        
    catch ME
        fprintf('[ERROR] compare_results: Error comparing %s: %s\n', cpp_filename, ME.message);
        fprintf(report_fid, '[ERROR] %s - %s\n', cpp_filename, ME.message);
    end
end

% 写入总体统计
fprintf(report_fid, '\n=====================================================\n');
fprintf(report_fid, 'SUMMARY STATISTICS\n');
fprintf(report_fid, '=====================================================\n');
fprintf(report_fid, 'Total files compared: %d\n', total_files);
fprintf(report_fid, 'Identical files: %d (%.2f%%)\n', identical_files, identical_files/total_files*100);
fprintf(report_fid, 'Files with differences: %d (%.2f%%)\n', diff_files, diff_files/total_files*100);
fprintf(report_fid, 'Missing MATLAB files: %d\n', missing_files);

if diff_files > 0 && ~isempty(all_diffs)
    fprintf(report_fid, '\nDIFFERENCE STATISTICS (across all differing files)\n');
    
    tl_diffs = [all_diffs.max_tl_diff];
    fprintf(report_fid, '  Max TL diff (dB): %.6f\n', max(tl_diffs));
    fprintf(report_fid, '  Mean TL diff (dB): %.6f\n', mean([all_diffs.mean_tl_diff]));
    
    range_diffs = [all_diffs.max_range_pct_diff];
    fprintf(report_fid, '  Max range diff (%%): %.6f\n', max(range_diffs));
    
    bearing_diffs = [all_diffs.max_bearing_pct_diff];
    fprintf(report_fid, '  Max bearing diff (%%): %.6f\n', max(bearing_diffs));
end

fprintf(report_fid, '=====================================================\n');
fprintf(report_fid, 'TOLERANCE THRESHOLDS\n');
fprintf(report_fid, '=====================================================\n');
fprintf(report_fid, 'TL_dB absolute tolerance: ±0.01 dB\n');
fprintf(report_fid, 'Range/Bearing relative tolerance: ±0.01%%\n');

fclose(report_fid);

% 输出到控制台
fprintf('[INFO] compare_results: Comparison completed\n');
fprintf('[INFO] compare_results: Report written to %s\n', report_file);

% 显示摘要
fprintf('\n========== COMPARISON SUMMARY ==========\n');
fprintf('Total files: %d\n', total_files);
fprintf('Identical: %d (%.2f%%)\n', identical_files, identical_files/total_files*100);
fprintf('With differences: %d (%.2f%%)\n', diff_files, diff_files/total_files*100);
fprintf('Missing: %d\n', missing_files);
fprintf('========================================\n');

end
