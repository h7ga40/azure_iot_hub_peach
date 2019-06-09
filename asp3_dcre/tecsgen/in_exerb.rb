# coding: utf-8
#
#  TECS Generator
#      Generator for TOPPERS Embedded Component System
#  
#   Copyright (C) 2008-2015 by TOPPERS Project
#--
#   上記著作権者は，以下の(1)～(4)の条件を満たす場合に限り，本ソフトウェ
#   ア（本ソフトウェアを改変したものを含む．以下同じ）を使用・複製・改
#   変・再配布（以下，利用と呼ぶ）することを無償で許諾する．
#   (1) 本ソフトウェアをソースコードの形で利用する場合には，上記の著作
#       権表示，この利用条件および下記の無保証規定が，そのままの形でソー
#       スコード中に含まれていること．
#   (2) 本ソフトウェアを，ライブラリ形式など，他のソフトウェア開発に使
#       用できる形で再配布する場合には，再配布に伴うドキュメント（利用
#       者マニュアルなど）に，上記の著作権表示，この利用条件および下記
#       の無保証規定を掲載すること．
#   (3) 本ソフトウェアを，機器に組み込むなど，他のソフトウェア開発に使
#       用できない形で再配布する場合には，次のいずれかの条件を満たすこ
#       と．
#     (a) 再配布に伴うドキュメント（利用者マニュアルなど）に，上記の著
#         作権表示，この利用条件および下記の無保証規定を掲載すること．
#     (b) 再配布の形態を，別に定める方法によって，TOPPERSプロジェクトに
#         報告すること．
#   (4) 本ソフトウェアの利用により直接的または間接的に生じるいかなる損
#       害からも，上記著作権者およびTOPPERSプロジェクトを免責すること．
#       また，本ソフトウェアのユーザまたはエンドユーザからのいかなる理
#       由に基づく請求からも，上記著作権者およびTOPPERSプロジェクトを
#       免責すること．
#  
#   本ソフトウェアは，無保証で提供されているものである．上記著作権者お
#   よびTOPPERSプロジェクトは，本ソフトウェアに関して，特定の使用目的
#   に対する適合性も含めて，いかなる保証も行わない．また，本ソフトウェ
#   アの利用により直接的または間接的に生じたいかなる損害に関しても，そ
#   の責任を負わない．
#  
#   $Id$
#++

# exerb 版で最初に読みこまれるファイル
# このファイルから generate.rb が読みこまれる

$IN_EXERB=true

# この変数は exerb 版のみ
$path = ENV[ 'PATH' ]

#=== cygwin のパスを Windows パスに変換
# cygpath コマンドを探して、見つかれば変換する
# 変換できなかった、または変換後のパスが見つからない場合、default を返す
def cygpath( path, default = nil )
  if ! $IN_EXERB then
    return default
  end
  if File.exist? path then
    return default
  end

  found = false
  $path.split( ";" ).each{ |path2|
    cygpath_path = "#{path2}\\cygpath.exe"
    if File.exist?( cygpath_path )  then
      path3 = nil
      dbgPrint "in_exerb: cygpath: found\n"
      IO.popen( "cygpath -m #{path}", "r"){ |io|
        path3 = io.gets.chomp!
      }
      dbgPrint "in_exerb: cygpath: '#{path}' => '#{path3}'\n"
      # File.stat( path3.to_s )

      if File.exist?( path3 )
        dbgPrint "in_exerb: cygpath: '#{path3}' found\n"
        return path3
      else
        break
      end
    end
  }
  dbgPrint "cygpath '#{path}' not found\n"
  return default
end

# gcc のパスを設定する
#  cygwin のあるバージョンでは gcc は gcc-3.exe または gcc-4.exe へのシンボリックリンクとなっている
#  exerb 版ではシンボリックリンクを辿れないため gcc の exe を探す
#  cygwin のリンクは、システム属性が付けられ、コマンドプロンプト上では dir /A:S を指定しないと表示されない
#  (cygwin のバージョンで実装が異なるかもしれない)
#  ファイルの先頭2バイトを読出し PE フォーマットのマジックに一致するか調べる
$gcc = "gcc"
$path.split( ";" ).each{ |path|
  found = false
  [ "gcc.exe", "gcc-3.exe", "gcc-4.exe" ].each{ |gcc|
    gcc_path = "#{path}\\#{gcc}"
    if File.exist?( gcc_path )  then
      begin
        magic = File.read( gcc_path, 2 )
        if magic == "MZ"                  # マジックをチェック
          $gcc = gcc
          found = true
          break
        end
      rescue
        # magic の読出しに失敗。無視
      end
    end
  }
  break if found
}

# cpp コマンドを設定する
$cpp = "#{$gcc} -E -D TECSGEN"

# exerb 版 TECS ジェネレータの実行を開始
require "tecsgen.rb"
