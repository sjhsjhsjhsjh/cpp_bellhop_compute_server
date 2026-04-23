function extract_and_write_tl(fileroot, config, output_dir)
% 从.shd文件中提取TL数据，按C++格式输出
%
% Usage: extract_and_write_tl(fileroot, config, output_dir)
%
% Input:
%   fileroot   - env文件的基名（不包含.shd扩展名）
%   config     - 由parse_env()返回的配置结构体
%   output_dir - 输出文件的目录
%
% Output:
%   在output_dir中创建 tl_<fileroot>.txt，包含：
%     首行：源坐标(m) + 接收器矩阵中心坐标(m)，精度4位小数
%     后续行：每个接收器点的 depth range bearing TL_dB
%
% Notes:
%   - 假定.shd文件位于当前目录或搜索路径中
%   - TL计算：TL_dB = -20 * log10(abs(pressure) + eps)

if ~exist(output_dir, 'dir')
    mkdir(output_dir);
end

% 读取.shd文件
shd_filename = [fileroot '.shd'];
fprintf('[INFO] extract_and_write_tl: Reading %s\n', shd_filename);

try
    [PlotTitle, PlotType, freqVec, freq0, atten, Pos, pressure] = read_shd(shd_filename);
catch ME
    error('extract_and_write_tl: Failed to read %s. Error: %s', shd_filename, ME.message);
end

fprintf('[INFO] extract_and_write_tl: read_shd completed\n');

% 获取维度信息
% pressure 维度应为 (Ntheta, Nsz, Nrz, Nrr)
% 由于单源单深度，实际有效数据是 pressure(:, 1, 1, :)
pressure_dim = size(pressure);
fprintf('[INFO] extract_and_write_tl: pressure dimensions: [%s]\n', num2str(pressure_dim));

% 提取单源单深度的数据
if length(pressure_dim) == 4
    % 标准4D压力矩阵
    p_extracted = squeeze(pressure(:, 1, 1, :));  % (Ntheta, Nrr)
elseif length(pressure_dim) == 2
    % 已经是2D矩阵
    p_extracted = pressure;
else
    error('extract_and_write_tl: Unexpected pressure matrix dimensions: %s', num2str(pressure_dim));
end

% 获取网格参数
ntheta = length(Pos.theta);
nrr = length(Pos.r.r);
depth = Pos.r.z;  % 单一值，接收器深度

fprintf('[INFO] extract_and_write_tl: Grid dimensions - Ntheta: %d, Nrr: %d, depth: %.1f m\n', ntheta, nrr, depth);

% 计算TL_dB
% pressure是复数，取绝对值后转换为dB
eps_val = 1e-37;  % 防止log(0)的小值
tl_db = -20 * log10(abs(p_extracted) + eps_val);

% 组织输出数据
% 迭代顺序：外循环距离(Nrr)，内循环方位角(Ntheta)
% 这样可以保证与C++版本的输出顺序一致
output_data = [];
for i_r = 1 : nrr
    for i_theta = 1 : ntheta
        range = Pos.r.r(i_r);           % 单位：m
        bearing = Pos.theta(i_theta);   % 单位：度
        tl_val = tl_db(i_theta, i_r);   % dB
        
        % 添加到输出数据矩阵：[depth, range, bearing, TL_dB]
        output_data = [output_data; depth, range, bearing, tl_val];
    end
end

% 输出文件路径
output_filename = fullfile(output_dir, sprintf('tl_%s.txt', fileroot));

% 打开文件进行写入
fid = fopen(output_filename, 'w');
if fid == -1
    error('extract_and_write_tl: Cannot open file for writing: %s', output_filename);
end

% 写入首行：源坐标 + 接收器矩阵中心坐标（精度4位小数）
fprintf(fid, '%.4f %.4f %.4f %.4f %.4f %.4f\n', ...
    config.sx, config.sy, config.sd, ...
    config.center_x, config.center_y, config.center_z);

% 写入后续行：depth, range, bearing, TL_dB（精度4位小数）
for i = 1 : size(output_data, 1)
    fprintf(fid, '%.4f %.4f %.4f %.4f\n', ...
        output_data(i, 1), output_data(i, 2), output_data(i, 3), output_data(i, 4));
end

fclose(fid);

fprintf('[INFO] extract_and_write_tl: Successfully written %s (total rows: %d)\n', ...
    output_filename, size(output_data, 1));

end
