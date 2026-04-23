function config = parse_env(env_filename)
% 解析env文件，提取Bellhop计算参数和接收器矩阵中心坐标
% 
% Usage: config = parse_env(env_filename)
% 
% Input:
%   env_filename - env文件的完整路径或文件名
%
% Output:
%   config - 结构体，包含：
%     .sx, .sy, .sd        - 源坐标 (单位：m，从env中的km转换)
%     .rd                   - 接收器深度 (单位：m)
%     .nr                   - 距离网格点数
%     .r_min, .r_max        - 距离范围 (单位：m，从env中的km转换)
%     .ntheta               - 方位角网格点数
%     .theta_min, .theta_max - 方位角范围 (单位：度)
%     .center_x, .center_y  - 接收器矩阵中心的笛卡尔坐标 (单位：m)
%     .center_z             - 接收器矩阵中心的深度 = rd (单位：m)
%
% Notes:
%   - env文件中的距离单位为km，本函数自动转换为m
%   - 接收器矩阵中心 = 源位置 + 中心距离 * (cos(中心方位角), sin(中心方位角))

if ~isfile(env_filename)
    error('parse_env: File not found: %s', env_filename);
end

% 初始化所有变量
sx = NaN;
sy = NaN;
sd = NaN;
rd = NaN;
nr = NaN;
r_min = NaN;
r_max = NaN;
ntheta = NaN;
theta_min = NaN;
theta_max = NaN;

% 读取env文件
fid = fopen(env_filename, 'r');
line_num = 0;

while ~feof(fid)
    line = fgetl(fid);
    if ~ischar(line)
        break;
    end
    
    line_num = line_num + 1;
    
    % 移除注释部分（! 之后的内容）
    comment_idx = find(line == '!', 1);
    if ~isempty(comment_idx)
        line_data = line(1:comment_idx-1);
    else
        line_data = line;
    end
    
    line_data = strtrim(line_data);
    
    % 跳过空行
    if isempty(line_data)
        continue;
    end
    
    % 根据行号或注释内容解析参数
    % 注意：需要跳过前几行（标题、频率等）
    % 根据env_1d15869ed451f6d9bd8a45e2c745e5b2.env的结构：
    % Line 1: TITLE
    % Line 2: FREQ (Hz)
    % Line 3: NMEDIA
    % Line 4: SSP options
    % Line 5: NPts, Sigma, bot depth
    % Lines 6+: SSP profile
    % ...后续是参数设置
    
    % 简化：根据注释关键词解析
    if contains(line, '! SX(1:NSX)') || contains(line, 'SX')
        vals = sscanf(line_data, '%f');
        if ~isempty(vals) && length(vals) >= 1
            sx = vals(1) * 1000;  % km -> m
        end
    elseif contains(line, '! SY(1:NSY)') || contains(line, 'SY')
        vals = sscanf(line_data, '%f');
        if ~isempty(vals) && length(vals) >= 1
            sy = vals(1) * 1000;  % km -> m
        end
    elseif contains(line, '! SD(1:NSD)')
        vals = sscanf(line_data, '%f');
        if ~isempty(vals) && length(vals) >= 1
            sd = vals(1);  % 已是m
        end
    elseif contains(line, '! RD(1:NRD)')
        vals = sscanf(line_data, '%f');
        if ~isempty(vals) && length(vals) >= 1
            rd = vals(1);  % 已是m
        end
    elseif contains(line, '! NR') && ~contains(line, 'NRD')
        vals = sscanf(line_data, '%f');
        if ~isempty(vals) && length(vals) >= 1
            nr = round(vals(1));
        end
    elseif contains(line, '! R(1:NR )')
        vals = sscanf(line_data, '%f');
        if ~isempty(vals) && length(vals) >= 2
            r_min = vals(1) * 1000;  % km -> m
            r_max = vals(2) * 1000;  % km -> m
        end
    elseif contains(line, '! Ntheta')
        vals = sscanf(line_data, '%f');
        if ~isempty(vals) && length(vals) >= 1
            ntheta = round(vals(1));
        end
    elseif contains(line, '! bearing angles') || contains(line, '! ALPHA')
        vals = sscanf(line_data, '%f');
        if ~isempty(vals) && length(vals) >= 2
            theta_min = vals(1);  % 度
            theta_max = vals(2);  % 度
        end
    end
end

fclose(fid);

% 验证必需的参数是否被成功解析
missing_params = {};
if isnan(sx), missing_params{end+1} = 'SX'; end
if isnan(sy), missing_params{end+1} = 'SY'; end
if isnan(sd), missing_params{end+1} = 'SD'; end
if isnan(rd), missing_params{end+1} = 'RD'; end
if isnan(nr), missing_params{end+1} = 'NR'; end
if isnan(r_min) || isnan(r_max), missing_params{end+1} = 'R range'; end
if isnan(ntheta), missing_params{end+1} = 'Ntheta'; end
if isnan(theta_min) || isnan(theta_max), missing_params{end+1} = 'bearing angles'; end

if ~isempty(missing_params)
    error('parse_env: Failed to parse required parameters: %s', strjoin(missing_params, ', '));
end

% 计算接收器矩阵中心
r_center = (r_min + r_max) / 2;
theta_center_rad = deg2rad((theta_min + theta_max) / 2);

center_x = sx + r_center * cos(theta_center_rad);
center_y = sy + r_center * sin(theta_center_rad);
center_z = rd;

% 返回配置结构体
config = struct();
config.sx = sx;
config.sy = sy;
config.sd = sd;
config.rd = rd;
config.nr = nr;
config.r_min = r_min;
config.r_max = r_max;
config.ntheta = ntheta;
config.theta_min = theta_min;
config.theta_max = theta_max;
config.center_x = center_x;
config.center_y = center_y;
config.center_z = center_z;

fprintf('[INFO] parse_env: Successfully parsed %s\n', env_filename);
fprintf('  Source: (%.1f m, %.1f m, %.1f m)\n', sx, sy, sd);
fprintf('  Receiver matrix center: (%.1f m, %.1f m, %.1f m)\n', center_x, center_y, center_z);
fprintf('  Distance range: %.1f - %.1f m (center: %.1f m)\n', r_min, r_max, r_center);
fprintf('  Bearing range: %.1f - %.1f degrees (center: %.1f deg)\n', theta_min, theta_max, (theta_min+theta_max)/2);

end
