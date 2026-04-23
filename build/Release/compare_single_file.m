function result = compare_single_file(cpp_file, matlab_file)
% 对比两个TL输出文件，返回详细的差异统计
%
% Usage: result = compare_single_file(cpp_file, matlab_file)
%
% Input:
%   cpp_file   - C++版本的tl_*.txt文件路径
%   matlab_file - MATLAB版本的tl_*.txt文件路径
%
% Output:
%   result - 结构体，包含对比统计信息：
%     .is_identical        - 是否完全相同
%     .cpp_lines           - C++版本的行数
%     .matlab_lines        - MATLAB版本的行数
%     .total_rows          - 数据行数（排除首行）
%     .matching_rows       - 匹配的行数
%     .max_tl_diff         - 最大TL_dB差值
%     .mean_tl_diff        - 平均TL_dB差值
%     .max_range_pct_diff  - 最大距离相对差异(%)
%     .max_bearing_pct_diff - 最大方位角相对差异(%)

% 读取C++文件
cpp_data = readmatrix(cpp_file);
cpp_lines = size(cpp_data, 1);

% 读取MATLAB文件
matlab_data = readmatrix(matlab_file);
matlab_lines = size(matlab_data, 1);

% 初始化统计变量
is_identical = false;
matching_rows = 0;
tl_diffs = [];
range_diffs_pct = [];
bearing_diffs_pct = [];

if cpp_lines == matlab_lines
    % 逐行对比
    % 列格式：depth, range, bearing, TL_dB
    
    % 第一行是源和接收器坐标，跳过（通常应该完全相同）
    for row = 2 : cpp_lines  % 从第2行开始（第1行是坐标）
        cpp_vals = cpp_data(row, :);
        matlab_vals = matlab_data(row, :);
        
        % 允许的浮点精度容差
        tol_tl = 0.01;  % dB
        tol_pct = 0.01; % %
        
        % 对比TL_dB（第4列）
        tl_diff = abs(cpp_vals(4) - matlab_vals(4));
        tl_diffs = [tl_diffs, tl_diff];
        
        % 对比range和bearing的相对差异
        if cpp_vals(2) > 0
            range_pct_diff = abs(cpp_vals(2) - matlab_vals(2)) / cpp_vals(2) * 100;
            range_diffs_pct = [range_diffs_pct, range_pct_diff];
        end
        
        if cpp_vals(3) > 0
            bearing_pct_diff = abs(cpp_vals(3) - matlab_vals(3)) / cpp_vals(3) * 100;
            bearing_diffs_pct = [bearing_diffs_pct, bearing_pct_diff];
        end
        
        % 判断该行是否匹配
        if tl_diff <= tol_tl && range_pct_diff <= tol_pct && bearing_pct_diff <= tol_pct
            matching_rows = matching_rows + 1;
        end
    end
    
    % 判断整体是否完全相同
    if matching_rows == cpp_lines - 1  % 排除第一行坐标
        is_identical = true;
    end
else
    % 行数不同，尽量对比前min(cpp_lines, matlab_lines)-1行
    min_lines = min(cpp_lines, matlab_lines);
    for row = 2 : min_lines
        cpp_vals = cpp_data(row, :);
        matlab_vals = matlab_data(row, :);
        
        tl_diff = abs(cpp_vals(4) - matlab_vals(4));
        tl_diffs = [tl_diffs, tl_diff];
        
        if cpp_vals(2) > 0
            range_pct_diff = abs(cpp_vals(2) - matlab_vals(2)) / cpp_vals(2) * 100;
            range_diffs_pct = [range_diffs_pct, range_pct_diff];
        end
        
        if cpp_vals(3) > 0
            bearing_pct_diff = abs(cpp_vals(3) - matlab_vals(3)) / cpp_vals(3) * 100;
            bearing_diffs_pct = [bearing_diffs_pct, bearing_pct_diff];
        end
        
        if tl_diff <= 0.01 && range_pct_diff <= 0.01 && bearing_pct_diff <= 0.01
            matching_rows = matching_rows + 1;
        end
    end
end

% 组织返回结果
result = struct();
result.is_identical = is_identical;
result.cpp_lines = cpp_lines;
result.matlab_lines = matlab_lines;
result.total_rows = cpp_lines - 1;  % 排除第一行坐标
result.matching_rows = matching_rows;

if ~isempty(tl_diffs)
    result.max_tl_diff = max(tl_diffs);
    result.mean_tl_diff = mean(tl_diffs);
else
    result.max_tl_diff = 0;
    result.mean_tl_diff = 0;
end

if ~isempty(range_diffs_pct)
    result.max_range_pct_diff = max(range_diffs_pct);
else
    result.max_range_pct_diff = 0;
end

if ~isempty(bearing_diffs_pct)
    result.max_bearing_pct_diff = max(bearing_diffs_pct);
else
    result.max_bearing_pct_diff = 0;
end

end
