//const CompressionWebpackPlugin = require('compression-webpack-plugin');

module.exports = {
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
    }
  }
};
