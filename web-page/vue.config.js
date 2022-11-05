//const CompressionWebpackPlugin = require('compression-webpack-plugin');

module.exports = {
  /*devServer: {
    proxy: {
        '/api': {
            target: 'http://192.168.4.1:80/',
            changeOrigin: true,
            ws: true
        }
    }
  },*/
  productionSourceMap: false,
  filenameHashing: false,
  css: {
    extract: false,
  },
  configureWebpack: {
    optimization: {
      splitChunks: false,
    },
    output: {
      filename: "js/[name].js",
      chunkFilename: "js/[name].js",
    },
    plugins: [
      /*new CompressionWebpackPlugin({
                filename: '[path][base].gz',
                test: /\.js(\?.*)?$/i,
                algorithm: 'gzip',
                threshold: 10240,
                minRatio: 0.8,
                deleteOriginalAssets: false
            })*/
    ],
  },
};
