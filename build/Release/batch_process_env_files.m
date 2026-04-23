function batch_process_env_files(env_dir, output_dir)
% 批量处理env文件，执行bellhop3d计算并提取TL数据
%
% Usage: batch_process_env_files(env_dir, output_dir)
%
% Input:
%   env_dir    - 包含.env文件的目录（默认：当前目录）
%   output_dir - 输出tl_*.txt文件的目录（默认：matlab_output）
%
% Example:
%   batch_process_env_files('build/Release', 'matlab_output')
%
% Notes:
%   - 扫描env_dir下的所有env_*.env文件
%   - 对每个env文件执行：parse_env → bellhop3d → extract_tl
%   - 实时输出处理进度和日志

if nargin < 1
    env_dir = '.';
end

if nargin < 2
    output_dir = 'matlab_output';
end

% 确保输出目录存在
if ~exist(output_dir, 'dir')
    mkdir(output_dir);
end

% 扫描env文件
fprintf('[INFO] batch_process_env_files: Scanning for .env files in %s\n', env_dir);
env_files = dir(fullfile(env_dir, 'env_*.env'));

if isempty(env_files)
    warning('batch_process_env_files: No .env files found in %s', env_dir);
    return;
end

fprintf('[INFO] batch_process_env_files: Found %d .env files\n', length(env_files));

% 处理每个env文件
total_files = length(env_files);
success_count = 0;
failed_count = 0;
failed_files = {};

for i = 1 : total_files
    env_file = env_files(i).name;
    env_filepath = fullfile(env_dir, env_file);
    
    % 提取fileroot（移除.env扩展名）
    [~, fileroot, ~] = fileparts(env_file);
    
    fprintf('\n[%d/%d] Processing: %s\n', i, total_files, env_file);
    
    try
        % Step 1: 解析env文件
        fprintf('  [1/3] Parsing env file...\n');
        config = parse_env(env_filepath);
        
        % Step 2: 执行bellhop3d计算
        fprintf('  [2/3] Running bellhop3d...\n');
        bellhop3d(fileroot);
        fprintf('  [2/3] bellhop3d completed\n');
        
        % Step 3: 提取TL数据并输出
        fprintf('  [3/3] Extracting TL data...\n');
        extract_and_write_tl(fileroot, config, output_dir);
        
        success_count = success_count + 1;
        fprintf('  ✓ Successfully completed\n');
        
    catch ME
        failed_count = failed_count + 1;
        failed_files{end+1} = env_file;
        fprintf('  ✗ FAILED: %s\n', ME.message);
        fprintf('     Error trace: %s\n', ME.stack(1).name);
    end
end

% 输出摘要
fprintf('\n');
fprintf('================== BATCH PROCESSING SUMMARY ==================\n');
fprintf('Total files:        %d\n', total_files);
fprintf('Successfully processed: %d\n', success_count);
fprintf('Failed:             %d\n', failed_count);

if failed_count > 0
    fprintf('\nFailed files:\n');
    for i = 1 : length(failed_files)
        fprintf('  - %s\n', failed_files{i});
    end
end

fprintf('=============================================================\n');

end
