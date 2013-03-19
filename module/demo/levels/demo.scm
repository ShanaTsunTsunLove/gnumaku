(define-module (demo levels demo)
  #:use-module (oop goops)
  #:use-module (gnumaku generics)
  #:use-module (gnumaku core)
  #:use-module (gnumaku assets)
  #:use-module (demo level)
  #:use-module (demo actor)
  #:use-module (demo player)
  #:use-module (demo enemies)
  #:export (<demo-level> make-demo-level))

(define-class <demo-level> (<level>))

(define bullet-sprites #f)

(define (make-demo-level player width height)
  (set! bullet-sprites (load-asset "bullets.png" 32 32 0 0))
  (let ((level (make <demo-level> #:player player #:width width #:height height
                    #:background (load-asset "space.png")
                    #:player-bullet-system (make-bullet-system 1000 bullet-sprites)
                    #:enemy-bullet-system (make-bullet-system 50000 bullet-sprites))))
    (set! (bullet-system player) (player-bullet-system level))
    (init-level level)
    level))

(define-method (run (level <demo-level>))
  #f)

;; (define (emit-test level)
;;   (coroutine
;;    (let loop ((system (level-enemy-bullet-system level))
;;               (rotate 0))
;;      (emit-circle system (/ (width level) 2) 100 30 12 (* -1 rotate) 100 0 -10 'large-orange)
;;      (level-wait level 15)
;;      (emit-circle system (/ (width level) 2) 100 30 12 rotate 100 0 10 'medium-blue)
;;      (level-wait level 15)
;;      (loop system (+ rotate 5)))))
